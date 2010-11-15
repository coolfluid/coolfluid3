// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for Zoltan load balancing library"

// boost
#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/mpi/collectives.hpp>

/*// for boost graph example
#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/tuple/tuple.hpp>

#include <boost/graph/use_mpi.hpp>
#include <boost/graph/distributed/mpi_process_group.hpp>
#include <boost/graph/distributed/adjacency_list.hpp>
#include <boost/graph/distributed/local_subgraph.hpp>
#include <boost/graph/parallel/distribution.hpp>
*/
// zoltan
#include <mpi.h>
#include <zoltan_cpp.h>

// coolfluid
#include "Common/ConfigObject.hpp"
#include "Common/Log.hpp"
#include "Common/String/Conversion.hpp"

#include "Common/MPI/PE.hpp"

#include "Math/MatrixTypes.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/ConnectivityData.hpp"
#include "Mesh/CFlexTable.hpp"




/*
#include <boost/foreach.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>

#define BOOST_FOREACH_ASSIGN_VAR(R, ROW, I, VAR)                                                                  \
  for (VAR = boost::fusion::at_c<I>(ROW); !BOOST_FOREACH_ID(_foreach_leave_outerloop);                            \
      BOOST_FOREACH_ID(_foreach_leave_outerloop) = true)

#define BOOST_FOREACH_FIELD(VARS, COL)                                                                            \
    BOOST_FOREACH_PREAMBLE()                                                                                      \
    if (boost::foreach_detail_::auto_any_t BOOST_FOREACH_ID(_foreach_col) = BOOST_FOREACH_CONTAIN(COL)) {} else   \
    if (boost::foreach_detail_::auto_any_t BOOST_FOREACH_ID(_foreach_cur) = BOOST_FOREACH_BEGIN(COL)) {} else     \
    if (boost::foreach_detail_::auto_any_t BOOST_FOREACH_ID(_foreach_end) = BOOST_FOREACH_END(COL)) {} else       \
    for (bool BOOST_FOREACH_ID(_foreach_continue) = true,                                                         \
        BOOST_FOREACH_ID(_foreach_leave_outerloop) = true;                                                  \
        BOOST_FOREACH_ID(_foreach_continue) && !BOOST_FOREACH_DONE(COL);                                    \
        BOOST_FOREACH_ID(_foreach_continue) ? BOOST_FOREACH_NEXT(COL) : (void)0)                            \
        if  (boost::foreach_detail_::set_false(BOOST_FOREACH_ID(_foreach_continue))) {} else                      \
        if  (boost::foreach_detail_::set_false(BOOST_FOREACH_ID(_foreach_leave_outerloop))) {} else               \
        BOOST_PP_SEQ_FOR_EACH_I(BOOST_FOREACH_ASSIGN_VAR, BOOST_FOREACH_DEREF(COL), VARS)                         \
        for (; !BOOST_FOREACH_ID(_foreach_continue); BOOST_FOREACH_ID(_foreach_continue) = true)

*/


#define PE_SERIALIZE(func)                                \
{																													\
	PE::instance().barrier();																\
	CFinfo.setFilterRankZero(false);												\
  for (Uint proc=0; proc<PE::instance().size(); ++proc)   \
  {                                                       \
    if (proc == PE::instance().rank())                    \
    {                                                     \
      func                                                \
    }                                                     \
    PE::instance().barrier();                             \
  }                                                       \
	CFinfo.setFilterRankZero(true);													\
	PE::instance().barrier();																\
}
        
using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace CF::Common::String;
using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////



typedef struct 
{
  int numMyVertices; /* total vertices in in my partition */
  int numAllNbors;   /* total number of neighbors of my vertices */
  int glb_nb_vertices;
  std::vector<int> globalID;     /* global ID of each of my vertices */
  std::vector<int> nborIdx;      /* nborIndex[i] is location of start of neighbors for vertex i */
  std::vector<int> nborGID;      /* nborGIDs[nborIndex[i]] is first neighbor of vertex i */
  std::vector<int> nbNbors;      /* nborGIDs[nborIndex[i]] is first neighbor of vertex i */
  std::vector<int> nborProc;     /* process owning each nbor in nborGID */
} GRAPH_DATA;

struct ZoltanTests_Fixture
{
  /// common setup for each test case
  ZoltanTests_Fixture()
  {
		// uncomment if you want to use arguments to the test executable
		m_argc = boost::unit_test::framework::master_test_suite().argc;
		m_argv = boost::unit_test::framework::master_test_suite().argv;
		
  }

  /// common tear-down for each test case
  ~ZoltanTests_Fixture()
  {

  }

  /// possibly common functions used on the tests below

	static int* to_ptr(std::vector<int>& vec)
	{
		if (vec.empty())
			return NULL;
		else
			return &vec[0];
	}
	

  static int get_number_of_objects(void *data, int *ierr)
  {
    *ierr = ZOLTAN_OK;
    GRAPH_DATA& graph = *(GRAPH_DATA *)data;
    return graph.globalID.size();
  }

  static void get_object_list(void *data, int sizeGID, int sizeLID,
                              ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                              int wgt_dim, float *obj_wgts, int *ierr)
  {
    GRAPH_DATA& graph = *(GRAPH_DATA *)data;
    
    *ierr = ZOLTAN_OK;

    // In this example, return the IDs of our objects, but no weights.
    // Zoltan will assume equally weighted objects.
    
    for (Uint i=0; i<graph.globalID.size(); i++){
      globalID[i] = graph.globalID[i];
      localID[i] = i;
    }
    return;
  }
  
  static void get_num_edges_list(void *data, int sizeGID, int sizeLID,
                                 int num_obj,
                                 ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                                 int *numEdges, int *ierr)
  {
    GRAPH_DATA& graph = *(GRAPH_DATA *)data;

    if ( (sizeGID != 1) || (sizeLID != 1) )
    {
      *ierr = ZOLTAN_FATAL;
      return;
    }

    if (num_obj != (int)graph.globalID.size())
    {
      *ierr = ZOLTAN_FATAL;
      return;
      
    }
    for (int i=0;  i < num_obj ; i++)
    {
      numEdges[i] = graph.nborIdx[i+1] - graph.nborIdx[i];
    }

    *ierr = ZOLTAN_OK;
    return;
  }

  static void get_edges_list(void *data, int sizeGID, int sizeLID,
          int num_obj, ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
          int *num_edges,
          ZOLTAN_ID_PTR nborGID, int *nborProc,
          int wgt_dim, float *ewgts, int *ierr)
  {
    GRAPH_DATA& graph = *(GRAPH_DATA *)data;
    *ierr = ZOLTAN_OK;

    if ( (sizeGID != 1) || (sizeLID != 1) || 
         (num_obj != (int)graph.globalID.size()) ||
         (wgt_dim != 0)){
      *ierr = ZOLTAN_FATAL;
      return;
    }
    
    for (Uint i=0; i<graph.nborGID.size(); ++i)
    {
      nborGID[i] = graph.nborGID[i];
      nborProc[i] = graph.nborProc[i];
    }
    
    //nborProc = NULL;
    return;
  }
    
  boost::shared_ptr<GRAPH_DATA> build_graph()
  {
    boost::shared_ptr<GRAPH_DATA> graph_ptr (new GRAPH_DATA);
    GRAPH_DATA& graph = *graph_ptr;
    switch (PE::instance().rank())
    {
      case 0:
      {
        graph.globalID = list_of( 1)( 2)( 3)( 4)( 5)( 6)( 7)( 8);
        graph.nborIdx = list_of ( 0)( 2)( 5)( 8)(11)(13)(16)(20)(24);
        graph.nborGID = list_of ( 2)( 6)
                                ( 1)( 3)( 7)
                                ( 2)( 8)( 4)
                                ( 3)( 9)( 5)
                                ( 4)(10)
                                ( 1)( 7)(11)
                                ( 6)( 2)( 8)(12)
                                ( 7)( 3)( 9)(13);
        graph.nborProc = list_of( 0)( 0)
                                ( 0)( 0)( 0)
                                ( 0)( 0)( 0)
                                ( 0)( 1)( 0)
                                ( 0)( 1)
                                ( 0)( 0)( 1)
                                ( 0)( 0)( 0)( 1)
                                ( 0)( 0)( 1)( 1);
        break;
      }
      case 1:
      {
        graph.globalID = list_of( 9)(10)(11)(12)(13)(14)(15)(16);
        graph.nborIdx = list_of ( 0)( 4)( 7)(10)(14)(18)(22)(25)(28);
        graph.nborGID = list_of ( 8)( 4)(10)(14)
                                ( 9)( 5)(15)
                                ( 6)(12)(16)
                                (11)( 7)(13)(17)
                                (12)( 8)(14)(18)
                                (13)( 9)(15)(19)
                                (14)(10)(20)
                                (11)(17)(21);
        graph.nborProc = list_of( 0)( 0)( 1)( 1)
                                ( 1)( 0)( 1)
                                ( 0)( 1)( 1)
                                ( 1)( 0)( 1)( 2)
                                ( 1)( 0)( 1)( 2)
                                ( 1)( 1)( 1)( 2)
                                ( 1)( 1)( 2)
                                ( 1)( 2)( 2);
        break;
      }
      case 2:
      {
     
        graph.globalID = list_of(17)(18)(19)(20)(21)(22)(23)(24)(25);
        graph.nborIdx = list_of ( 0)( 4)( 8)(12)(15)(17)(20)(23)(26)(28);
        graph.nborGID = list_of (16)(12)(18)(22)
                                (17)(13)(19)(23)
                                (18)(14)(20)(24)
                                (19)(15)(25)
                                (16)(22)
                                (21)(17)(23)
                                (22)(18)(24)
                                (23)(19)(25)
                                (24)(20);
        graph.nborProc = list_of( 1)( 1)( 2)( 2)
                                ( 2)( 1)( 2)( 2)
                                ( 2)( 1)( 2)( 2)
                                ( 2)( 1)( 2)
                                ( 1)( 2)
                                ( 2)( 2)( 2)
                                ( 2)( 2)( 2)
                                ( 2)( 2)( 2)
                                ( 2)( 2);
        break;
      }
    }
    graph.glb_nb_vertices = 25;
    return graph_ptr;
  }

  boost::shared_ptr<GRAPH_DATA> build_element_node_graph()
  {
    boost::shared_ptr<GRAPH_DATA> graph_ptr (new GRAPH_DATA);
    GRAPH_DATA& graph = *graph_ptr;
    Uint a=26, b=27, c=28, d=29, e=30, f=31, g=32, h=33, i=34, j=35, k=36, l=37, m=38, n=39, o=40, p=41;
    switch (PE::instance().rank())
    {
      case 0:
      {
        graph.globalID = list_of(1)(2)(3)(4)(5)(6)(7)(8) (a)(b)(c)(d)(e);

        graph.nborGID = list_of (a)                      // 1
                                (a)(b)                   // 2
                                (b)(c)                   // 3
                                (c)(d)                   // 4
                                (d)                      // 5
                                (a)(e)                   // 6
                                (a)(b)(e)(f)             // 7
                                (b)(c)(f)(g)             // 8
        
                                (1)(2)(6)(7)             // a
                                (2)(3)(7)(8)             // b
                                (3)(4)(8)(9)             // c
                                (4)(5)(9)(10)            // d
                                (6)(7)(11)(12);          // e
        
        graph.nbNbors = list_of (1)(2)(2)(2)(1)(2)(4)(4) (4)(4)(4)(4)(4);
        break;
      }
      case 1:
      {
        graph.globalID = list_of(9)(10)(11)(12)(13)(14)(15)(16) (f)(g)(h)(i)(j);

        graph.nborGID = list_of (c)(d)(g)(h)             // 9
                                (d)(h)                   // 10
                                (e)(i)                   // 11
                                (e)(f)(i)(j)             // 12
                                (f)(g)(j)(k)             // 13
                                (g)(h)(k)(l)             // 14
                                (h)(l)                   // 15
                                (i)(m)                   // 16
        
                                (7)(8)(12)(13)           // f
                                (8)(9)(13)(14)           // g
                                (9)(10)(14)(15)          // h
                                (11)(12)(16)(17)         // i
                                (12)(13)(17)(18);        // j
        
        graph.nbNbors = list_of (4)(2)(2)(4)(4)(4)(2)(2) (4)(4)(4)(4)(4);
        break;
      }
      case 2:
      {
        graph.globalID.resize(15);
        graph.globalID = list_of(17)(18)(19)(20)(21)(22)(23)(24)(25) (k)(l)(m)(n)(o)(p);

        graph.nborGID = list_of (i)(j)(m)(n)             // 17
                                (j)(k)(n)(o)             // 18
                                (k)(l)(o)(p)             // 19
                                (l)(p)                   // 20
                                (m)                      // 21
                                (m)(n)                   // 22
                                (n)(o)                   // 23
                                (o)(p)                   // 24
                                (p)                      // 25
        
                                (13)(14)(18)(19)         // k
                                (14)(15)(19)(20)         // l
                                (16)(17)(21)(22)         // m
                                (17)(18)(22)(23)         // n
                                (18)(19)(23)(24)         // o
                                (19)(20)(24)(25);        // p
        
        graph.nbNbors = list_of (4)(4)(4)(2)(1)(2)(2)(2)(1) (4)(4)(4)(4)(4)(4);
        break;
      }
    }    
    
    graph.nborIdx.resize(graph.nbNbors.size()+1);
    graph.nborIdx[0]=0;
    for (Uint i=1; i<graph.nborIdx.size(); ++i)
    {
      graph.nborIdx[i] = graph.nborIdx[i-1]+graph.nbNbors[i-1];
    }

    graph.nborProc.resize(graph.nborGID.size());
    for(Uint i=0; i<graph.nborProc.size(); ++i)
    { 
      if      (graph.nborGID[i] <= 8)  graph.nborProc[i] = 0;
      else if (graph.nborGID[i] <= 16) graph.nborProc[i] = 1;
      else if (graph.nborGID[i] <= 25) graph.nborProc[i] = 2;
      else if (graph.nborGID[i] <= 30) graph.nborProc[i] = 0;
      else if (graph.nborGID[i] <= 35) graph.nborProc[i] = 1;
      else if (graph.nborGID[i] <= 41) graph.nborProc[i] = 2;
      else throw BadValue(FromHere(), "globalID out of bounds");
    }
    
    
    if(PE::instance().rank()==2)
    {
      CFinfo.setFilterRankZero(false);
      for (Uint i=0; i<graph.globalID.size(); ++i)
      CFinfo << graph.globalID[i] << CFendl;
      CFinfo.setFilterRankZero(true);
    }
    PE::instance().barrier();
    
    graph.glb_nb_vertices = 41;
    
    return graph_ptr;
  }
  
	/* Draw the partition assignments of the objects */

  static void showGraphPartitions(GRAPH_DATA& graph, std::vector<int>& parts)
  {        
    std::vector<int> part_assign_on_this_proc(graph.glb_nb_vertices);
    std::vector<int> part_assign(graph.glb_nb_vertices);
    
    for (Uint i=0; i < parts.size(); i++){
      part_assign_on_this_proc[graph.globalID[i]-1] = parts[i];
    }

    boost::mpi::reduce(PE::instance(), to_ptr(part_assign_on_this_proc),  part_assign.size() , to_ptr(part_assign), boost::mpi::maximum<int>(),0);
    
    
    for (Uint i=0; i < part_assign.size(); i++){
      CFinfo << i+1 << "  -->  " << part_assign[i] << CFendl;;
    }
    
  int i, j, part, cuts, prevPart=-1;
  float imbal, localImbal, sum;
  std::vector<int> partCount(PE::instance().size());

    if (PE::instance().rank() == 0){

      cuts = 0;

      for (i=20; i >= 0; i-=5){
        for (j=0; j < 5; j++){
          part = part_assign[i + j];
          partCount[part]++;
          if (j > 0){
            if (part == prevPart){
              printf("-----%d",part);
            }
            else{
              printf("--x--%d",part);
              cuts++;
              prevPart = part;
            }
          }
          else{
            printf("%d",part);
            prevPart = part;
          }
        }
        printf("\n");
        if (i > 0){
          for (j=0; j < 5; j++){
            if (part_assign[i+j] != part_assign[i+j-5]){
              printf("x     ");
              cuts++;
            }
            else{
              printf("|     ");
            }
          }
          printf("\n");
        }
      }
      printf("\n");

      for (sum=0, i=0; i < (int)PE::instance().size(); i++){
        sum += partCount[i];
      }
      imbal = 0;
      for (i=0; i < (int)PE::instance().size(); i++){
        /* An imbalance measure.  1.0 is perfect balance, larger is worse */
        localImbal = (PE::instance().size() * partCount[i]) / sum;
        if (localImbal > imbal) imbal = localImbal;
      }

      printf("Object imbalance (1.0 perfect, larger numbers are worse): %f\n",imbal);
      printf("Total number of edge cuts: %d\n\n",cuts);

    }

  }
  
	//////////////////////////////////////////////////////////////////////

	enum ObjectIndex { IDX=0, COMP=1 };

  static int get_number_of_objects_mesh(void *data, int *ierr)
  {
    *ierr = ZOLTAN_OK;
    CMesh& mesh = *(CMesh *)data;
    // Uint nb_nodes = get_component_typed<CRegion>(mesh).recursive_nodes_count();
    
    Uint nb_nodes = 0;
    BOOST_FOREACH(const CList<bool>& is_ghost, recursive_filtered_range_typed<CList<bool> >(mesh,IsComponentTag("is_ghost")))
    {
      BOOST_FOREACH(const bool is_node_ghost, is_ghost.array())
      {
        if (!is_node_ghost)
          ++nb_nodes;
      }
    }
    
    
    //CFLogVar(nb_nodes);
    Uint nb_elems = get_component_typed<CRegion>(mesh).recursive_elements_count();
    //CFLogVar(nb_elems);
		CFLogVar(nb_nodes+nb_elems);
    return nb_nodes+nb_elems;
  }
	
	
	
	
	
	
	
	
	
	
	

  static void get_object_list_mesh(void *data, int sizeGID, int sizeLID,
                              ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                              int wgt_dim, float *obj_wgts, int *ierr)
  {
    CMesh& mesh = *(CMesh *)data;
    
    *ierr = ZOLTAN_OK;
		
		//if ( (sizeGID != 1) || (sizeLID != 1) || 
		//		(wgt_dim != 0)){
    //  *ierr = ZOLTAN_FATAL;
    //  return;
    //}
		

    // In this example, return the IDs of our objects, but no weights.
    // Zoltan will assume equally weighted objects.
        
    //ZOLTAN_ID_PTR global_idx = globalID;
    //ZOLTAN_ID_PTR local_idx = localID;
		
		int* glb_idx;
		int* loc_idx;
		
		Uint node_start_idx = mesh.get_child("temporary_partition_info")->property("node_start_idx").value<Uint>();
		Uint elem_start_idx = mesh.get_child("temporary_partition_info")->property("elem_start_idx").value<Uint>();
		
		// Loop over the nodes of the mesh
		// -------------------------------

		Uint zoltan_idx = 0;
		Uint component_idx=0; // index counting the number of components that will be traversed
    BOOST_FOREACH(const CList<Uint>& global_node_indices, recursive_filtered_range_typed<CList<Uint> >(mesh,IsComponentTag("global_node_indices")))
    {
			CFinfo << "node comp #"<<component_idx<< " path = " << global_node_indices.get_parent()->full_path().string() << CFendl;

      const CList<bool>& is_ghost = *global_node_indices.get_parent()->get_child_type<CList<bool> >("is_ghost");
      
      Uint idx=0;
      BOOST_FOREACH(const Uint glb_node_idx, global_node_indices.array())
      {
        if (!is_ghost[idx])
        {
					glb_idx = (int *)(globalID + zoltan_idx * sizeGID);
					loc_idx = (int *)(localID + zoltan_idx * sizeLID);

					glb_idx[IDX]  = node_start_idx + glb_node_idx;
          loc_idx[IDX]  = idx;
          loc_idx[COMP] = component_idx;
  				
          ++zoltan_idx;
        }
				++idx;
      }
			++component_idx;
    }
    
		// Loop over the elements of the mesh
		// -------------------------------
    
    BOOST_FOREACH(const CList<Uint>& global_element_indices, recursive_filtered_range_typed<CList<Uint> >(mesh,IsComponentTag("global_element_indices")))
    {
			CFinfo << "elem comp #"<<component_idx<< " path = " << global_element_indices.get_parent()->full_path().string() << CFendl;
			Uint idx = 0;
      BOOST_FOREACH(const Uint glb_elm_idx, global_element_indices.array())
      {
				glb_idx = (int *)(globalID + zoltan_idx * sizeGID);				 
				loc_idx = (int *)(localID + zoltan_idx * sizeLID);

        //CFinfo << "elem: GID = " << elem_start_idx + glb_elm_idx << CFendl;
				glb_idx[IDX]  = elem_start_idx + glb_elm_idx;				
				loc_idx[IDX]  = idx;
				loc_idx[COMP] = component_idx;
				
        ++zoltan_idx;
				++idx;
      }
			++component_idx;
    }
		
		
		int error;
    Uint tot_nb_objects =  get_number_of_objects_mesh(data, &error);

		if (zoltan_idx > tot_nb_objects)
		{
			//throw BadValue(FromHere(),"zoltan_idx exceeds tot_nb_objects");
			*ierr = ZOLTAN_FATAL;
		}

    return;
  }
  
  static void get_num_edges_list_mesh(void *data, int sizeGID, int sizeLID,
                                 int num_obj,
                                 ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                                 int *numEdges, int *ierr)
  {
    CMesh& mesh = *(CMesh *)data;
    
    *ierr = ZOLTAN_OK;
    
    int error;
    Uint tot_nb_objects =  get_number_of_objects_mesh(data, &error);
  
    Uint zoltan_idx = 0;
    BOOST_FOREACH(const CFlexTable& node_to_glb_elm, recursive_filtered_range_typed<CFlexTable>(mesh,IsComponentTag("glb_elem_connectivity")))
    {
      if (zoltan_idx >= tot_nb_objects)
			{
				//throw BadValue(FromHere(),"zoltan_idx exceeds tot_nb_objects");
				*ierr = ZOLTAN_FATAL;
				return;
			}
      
      const CList<bool>& is_ghost = *node_to_glb_elm.get_parent()->get_child_type<CList<bool> >("is_ghost");
      for (Uint i=0; i<node_to_glb_elm.size(); ++i)
      {
        if (!is_ghost[i])
        {
          CFlexTable::ConstRow glb_elms = node_to_glb_elm[i];
          numEdges[zoltan_idx]=glb_elms.size();
          zoltan_idx++;
        }
      }
    }
        
    BOOST_FOREACH(const CElements& elements, recursive_range_typed<CElements>(get_component_typed<CRegion>(mesh)))
    {
      const CTable& conn_table = elements.connectivity_table();
      BOOST_FOREACH(CTable::ConstRow local_nodes, conn_table.array())
      {
        if (zoltan_idx >= tot_nb_objects)
				{
					//throw BadValue(FromHere(),"zoltan_idx exceeds tot_nb_objects");
					*ierr = ZOLTAN_FATAL;
					return;
				}
        numEdges[zoltan_idx]=local_nodes.size();
        zoltan_idx++;
      }
    }
    
    *ierr = ZOLTAN_OK;
    return;
  }

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
  static void get_edges_list_mesh(void *data, int sizeGID, int sizeLID,
          int num_obj, ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
          int *num_edges,
          ZOLTAN_ID_PTR nborGID, int *nborProc,
          int wgt_dim, float *ewgts, int *ierr)
  {
    CMesh& mesh = *(CMesh *)data;
    *ierr = ZOLTAN_OK;

    //nborProc = NULL;
    
    // if ( (sizeGID != 1) || (sizeLID != 1) || 
    //          (wgt_dim != 0)){
    //       *ierr = ZOLTAN_FATAL;
    //       return;
    //     }
    
		
		ZOLTAN_ID_PTR nbor_glb_idx = nborGID;
    int* nbor_proc = nborProc;
		
		Uint node_start_idx = mesh.get_child("temporary_partition_info")->property("node_start_idx").value<Uint>();
		Uint elem_start_idx = mesh.get_child("temporary_partition_info")->property("elem_start_idx").value<Uint>();
    
		CFLogVar(node_start_idx);
		CFLogVar(elem_start_idx);
		
    Uint num_edges_from_nodes=0;
    Uint num_edges_from_elems=0;

		BOOST_FOREACH(const CFlexTable& node_to_glb_elm, recursive_filtered_range_typed<CFlexTable>(mesh,IsComponentTag("glb_elem_connectivity")))
    {
      const CList<bool>& is_ghost = *node_to_glb_elm.get_parent()->get_child_type<CList<bool> >("is_ghost");
      for (Uint i=0; i<node_to_glb_elm.size(); ++i)
      {
        if (!is_ghost[i])
        {
          CFlexTable::ConstRow glb_elms = node_to_glb_elm[i];
          for (Uint j=0; j<glb_elms.size(); ++j)
          {
						num_edges_from_nodes++;
						*nbor_glb_idx++ = elem_start_idx + glb_elms[j];
						//CFinfo << "   " << i << " --> " << elem_start_idx + glb_elms[j] << CFendl;
            *nbor_proc++ = glb_elms[j] < 8 ? 0 : 1;
          }
        }
      }
    }

    BOOST_FOREACH(const CElements& elements, recursive_range_typed<CElements>(get_component_typed<CRegion>(mesh)))
    {
      const CArray& coordinates = elements.coordinates();
      const CList<Uint>& glb_node_idx = get_tagged_component_typed< CList<Uint> > (coordinates,"global_node_indices");
			//const CList<Uint>& glb_elm_idx = get_tagged_component_typed< CList<Uint> > (elements,"global_element_indices");

      const CTable& conn_table = elements.connectivity_table();
			Uint loc_elm_idx = 0;
      BOOST_FOREACH(CTable::ConstRow local_nodes, conn_table.array())
      {        
        BOOST_FOREACH(const Uint loc_node, local_nodes)
        {					
					num_edges_from_elems++;
					*nbor_glb_idx++ = node_start_idx + glb_node_idx[loc_node];
					//CFinfo << "   " << glb_elm_idx[loc_elm_idx] << " --> " << node_start_idx + glb_node_idx[loc_node] << CFendl;

					*nbor_proc++ = glb_node_idx[loc_node] < 8 ? 0 : 1;
        }
				++loc_elm_idx;
      }
    }
    
    Uint total_num_edges_from_nodes = boost::mpi::all_reduce(PE::instance(),num_edges_from_nodes,std::plus<Uint>());
    Uint total_num_edges_from_elems = boost::mpi::all_reduce(PE::instance(),num_edges_from_elems,std::plus<Uint>());

    if (total_num_edges_from_nodes != total_num_edges_from_elems)
      *ierr = ZOLTAN_FATAL;
				
    CFLogVar(num_edges_from_nodes);
    CFLogVar(total_num_edges_from_nodes);
    CFLogVar(num_edges_from_elems);
    CFLogVar(total_num_edges_from_elems);
  }
    
  
	/* Draw the partition assignments of the objects */

  static void showMeshPartitions(CMesh& mesh, std::vector<int>& parts)
  {
    Uint nb_nodes = mesh.property("nb_nodes").value<Uint>();
    Uint nb_cells = mesh.property("nb_cells").value<Uint>();
    Uint tot_num_objs = nb_nodes + nb_cells;
    
    std::vector<int> part_assign_on_this_proc(tot_num_objs);
    std::vector<int> part_assign(tot_num_objs);
    
    CList<Uint>& global_id = *mesh.get_child_type<CList<Uint> >("global_graph_id");
    for (Uint i=0; i < parts.size(); i++)
    {
      part_assign_on_this_proc[global_id[i]] = parts[i];
    }

    boost::mpi::reduce(PE::instance(), to_ptr(part_assign_on_this_proc),  part_assign.size() , to_ptr(part_assign), boost::mpi::maximum<int>(),0);
    
    
    for (Uint i=0; i < part_assign.size(); i++)
    {
      if (i<nb_nodes)
        CFinfo << "node ["<< i <<"]  -->  " << part_assign[i] << CFendl;
      else
        CFinfo << "elem ["<< i-nb_nodes <<"]  -->  " << part_assign[i] << CFendl;
    }
  }
	
	
	
	/* Application defined query functions for migrating */
	
	static void get_message_sizes(void *data, int gidSize, int lidSize, int num_ids,
																ZOLTAN_ID_PTR globalIDs, ZOLTAN_ID_PTR localIDs, int *sizes, int *ierr)
	{
	  CFinfo << "++++++++++++++++++++++++++++++++++ get_message_sizes"<<CFendl;
    
		CMesh& mesh = *(CMesh *)data;
		*ierr = ZOLTAN_OK;
		
		std::vector<Component::ConstPtr> components;
		BOOST_FOREACH(const CArray& coordinates, 
									recursive_filtered_range_typed<CArray>(mesh,IsComponentTag("coordinates")))
			components.push_back(coordinates.get());
    BOOST_FOREACH(const CElements& elements, recursive_range_typed<CElements>(mesh))
			components.push_back(elements.get());
		
		std::vector<CFlexTable::ConstPtr> list_of_node_to_glb_elm;
		BOOST_FOREACH(const CFlexTable& node_to_glb_elm, recursive_filtered_range_typed<CFlexTable>(mesh,IsComponentTag("glb_elem_connectivity")))
      list_of_node_to_glb_elm.push_back(node_to_glb_elm.get_type<CFlexTable>());
		
		//Uint node_start_idx = mesh.get_child("temporary_partition_info")->property("node_start_idx").value<Uint>();
		Uint elem_start_idx = mesh.get_child("temporary_partition_info")->property("elem_start_idx").value<Uint>();

		for (int i=0; i < num_ids; i++) 
		{
			int* loc_id = (int*)(localIDs+i*lidSize);
			int* glb_id = (int*)(globalIDs+i*gidSize);

			if (glb_id[IDX] < (int) elem_start_idx)
			{
				sizes[i] = sizeof(Uint) // component index
				         + sizeof(Real) * components[loc_id[COMP]]->get_type<CArray>()->row_size() // coordinates
                 + sizeof(Uint) * (1+list_of_node_to_glb_elm[loc_id[COMP]]->row_size(loc_id[IDX])); // global element indices that need this node
			}
			else
			{
				sizes[i] = 0;
			}

		}
	}
	
	
	static void pack_object_messages(void *data, int gidSize, int lidSize, int num_ids,
																	 ZOLTAN_ID_PTR globalIDs, ZOLTAN_ID_PTR localIDs, int *dests, int *sizes, int *idx, char *buf, int *ierr)
	{
		
    CFinfo << "++++++++++++++++++++++++++++++++++ pack_object_messages"<<CFendl;
		CMesh& mesh = *(CMesh *)data;
		*ierr = ZOLTAN_OK;
		
    Uint* component_number;
		Real* coord_row_buf;
    Uint* glb_elm_idx_buf;
    
		std::vector<Component::Ptr> components;
		BOOST_FOREACH(CArray& coordinates, 
									recursive_filtered_range_typed<CArray>(mesh,IsComponentTag("coordinates")))
			components.push_back(coordinates.get());
			
		std::vector<CFlexTable::ConstPtr> list_of_node_to_glb_elm;
		BOOST_FOREACH(const CFlexTable& node_to_glb_elm, recursive_filtered_range_typed<CFlexTable>(mesh,IsComponentTag("glb_elem_connectivity")))
      list_of_node_to_glb_elm.push_back(node_to_glb_elm.get_type<CFlexTable>());
		
		std::vector<CList<bool>::Ptr> list_of_is_ghost;
		BOOST_FOREACH(CList<bool>& is_ghost, recursive_filtered_range_typed<CList<bool> >(mesh,IsComponentTag("is_ghost")))
      list_of_is_ghost.push_back(is_ghost.get_type<CList<bool> >());		
		
		//Uint node_start_idx = mesh.get_child("temporary_partition_info")->property("node_start_idx").value<Uint>();
		Uint elem_start_idx = mesh.get_child("temporary_partition_info")->property("elem_start_idx").value<Uint>();

  	for (int id=0; id < num_ids; id++) 
		{
			int* loc_id = (int*)(localIDs+id*lidSize);
			int* glb_id = (int*)(globalIDs+id*gidSize);
		
			if (glb_id[IDX] < (int) elem_start_idx)
			{	
        component_number = (Uint *)(buf + idx[id]);
        *component_number++ = loc_id[COMP];

				coord_row_buf = (Real *)(component_number);
				BOOST_FOREACH(const Real& coord, components[loc_id[COMP]]->get_type<CArray>()->array()[loc_id[IDX]])
					*coord_row_buf++ = coord;
			
        glb_elm_idx_buf = (Uint *)(coord_row_buf);
        *glb_elm_idx_buf++ = list_of_node_to_glb_elm[loc_id[COMP]]->row_size(loc_id[IDX]);
				BOOST_FOREACH(const Uint& glb_elem_idx, list_of_node_to_glb_elm[loc_id[COMP]]->array()[loc_id[IDX]])
					*glb_elm_idx_buf++ = glb_elem_idx;

			  // mark this node as ghost
        list_of_is_ghost[loc_id[COMP]]->array()[loc_id[IDX]] = true;
			}
		}
	}
	
	static void mid_migrate(void *data, int gidSize, int lidSize,
													int numImport, ZOLTAN_ID_PTR importGlobalID, ZOLTAN_ID_PTR importLocalID, int *importProc, int *importPart,
													int numExport, ZOLTAN_ID_PTR exportGlobalID, ZOLTAN_ID_PTR exportLocalID, int *exportProc, int *exportPart,
													int *ierr)
	{
		*ierr = ZOLTAN_OK;
    CFinfo << "++++++++++++++++++++++++++++++++++ mid_migrate"<<CFendl;

	}
	static void unpack_object_messages(void *data, int gidSize, int num_ids,
																		 ZOLTAN_ID_PTR globalIDs, int *sizes, int *idx, char *buf, int *ierr)
	{		
	  CFinfo << "++++++++++++++++++++++++++++++++++ unpack_object_messages"<<CFendl;
    
		CMesh& mesh = *(CMesh *)data;
		*ierr = ZOLTAN_OK;

		std::vector< boost::shared_ptr<CArray::Buffer> > coordinates_buffer;
    std::vector< boost::shared_ptr<CList<bool>::Buffer> > is_ghost_buffer;
    std::vector< boost::shared_ptr<CList<Uint>::Buffer> > glb_node_indices_buffer;
    std::vector< boost::shared_ptr<CFlexTable::Buffer> > node_to_glb_elms_buffer;
		
		
		BOOST_FOREACH(CArray& coords, 
									recursive_filtered_range_typed<CArray>(mesh,IsComponentTag("coordinates")))
		{
		  coordinates_buffer.push_back( boost::shared_ptr<CArray::Buffer> (new CArray::Buffer(coords.get_type<CArray>()->create_buffer())));
			is_ghost_buffer.push_back( boost::shared_ptr<CList<bool>::Buffer> (new CList<bool>::Buffer(get_tagged_component_typed<CList<bool> >(coords,"is_ghost").create_buffer())));
			glb_node_indices_buffer.push_back( boost::shared_ptr<CList<Uint>::Buffer> (new CList<Uint>::Buffer(get_tagged_component_typed<CList<Uint> >(coords,"global_node_indices").create_buffer())));
			node_to_glb_elms_buffer.push_back( boost::shared_ptr<CFlexTable::Buffer> (new CFlexTable::Buffer(get_tagged_component_typed<CFlexTable>(coords,"glb_elem_connectivity").create_buffer())));
		}

    Uint comp_idx;
    
    Uint* component_number;
		Real* coord_row;
		Uint* glb_elm_idx_buf;
    
    std::vector<Real> coord_vec(2);
		 CFLogVar(num_ids);
		 for (int id=0; id<num_ids; ++id)
		 {
			 CFinfo << "receiving package with global id " << globalIDs[IDX + id*gidSize] << CFendl;
			 CFinfo << "    size = " << sizes[id]  << CFendl;
			 if (sizes[id] > 0)
			 {
			   component_number = (Uint *)(buf + idx[id]);
         comp_idx = *component_number++;
         CFinfo <<"#"<<comp_idx;
         
				 coord_row = (Real *)(component_number);
         coord_vec[0] = *coord_row++;
         coord_vec[1] = *coord_row++;
				 CFinfo << "    ( " << coord_vec[0] << " , " << coord_vec[1] << " )" << CFendl;
         CFinfo << "adding new coord at idx " << coordinates_buffer[comp_idx]->add_row(coord_vec) << CFendl;
         is_ghost_buffer[comp_idx]->add_row(false);
         
         glb_elm_idx_buf = (Uint *)(coord_row);
         std::vector<Uint> elems(*glb_elm_idx_buf++);
         for (Uint i=0; i<elems.size(); ++i)
           elems[i] = *glb_elm_idx_buf++;
         CFinfo << "adding glb elem indexes at idx " << node_to_glb_elms_buffer[comp_idx]->add_row(elems) << CFendl;
         
         CFinfo << "adding glb node index at idx " << glb_node_indices_buffer[comp_idx]->add_row(globalIDs[IDX + id*gidSize]) << CFendl;
         
         
			 }
		 }
		
	}
	
	
	void rm_ghost_nodes(CMesh& mesh)
  {
    BOOST_FOREACH(CArray& coordinates, recursive_filtered_range_typed<CArray>(mesh,IsComponentTag("coordinates")))
    {
      CList<bool>& is_ghost = get_tagged_component_typed< CList<bool> >(coordinates,"is_ghost");
      CList<Uint>& global_node_indices = get_tagged_component_typed< CList<Uint> >(coordinates,"global_node_indices");
      CFlexTable& glb_elem_connectivity = get_named_component_typed< CFlexTable >(coordinates,"glb_elem_connectivity");

      CFLogVar(coordinates.size());
      CFLogVar(is_ghost.size());
      CFLogVar(global_node_indices.size());
      CFLogVar(glb_elem_connectivity.size());


      CList<bool>::Buffer buffer_is_ghost = is_ghost.create_buffer();
      CList<Uint>::Buffer buffer_global_node_indices = global_node_indices.create_buffer();
      CFlexTable::Buffer buffer_glb_elem_connectivity = glb_elem_connectivity.create_buffer();
      
      CArray::Buffer buffer_coordinates = coordinates.create_buffer();
      for (Uint i=0; i<coordinates.size(); ++i)
      {
        if (is_ghost[i])
        {
          buffer_is_ghost.rm_row(i);
          buffer_global_node_indices.rm_row(i);
          buffer_coordinates.rm_row(i);
          buffer_glb_elem_connectivity.rm_row(i);
        }
      }
    }
    
    BOOST_FOREACH(const CArray& coordinates, recursive_filtered_range_typed<CArray>(mesh,IsComponentTag("coordinates")))
    {
      const CList<bool>& is_ghost = get_tagged_component_typed< CList<bool> >(coordinates,"is_ghost");
      const CList<Uint>& global_node_indices = get_tagged_component_typed< CList<Uint> >(coordinates,"global_node_indices");
      const CFlexTable& glb_elem_connectivity = get_named_component_typed< CFlexTable >(coordinates,"glb_elem_connectivity");
      
      
      CFLogVar(coordinates.size());
      CFLogVar(is_ghost.size());
      CFLogVar(global_node_indices.size());
      CFLogVar(glb_elem_connectivity.size());  
    }    
    
  }
	
	int m_argc;
	char** m_argv;
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( ZoltanTests_TestSuite, ZoltanTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
	PE::instance().init(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Zoltan_tutorial_construction )
{
	
	if (PE::instance().size() != 3)
	{
    CFinfo << "must run this testcase with 3 processors" << CFendl;
	}
	else
	{
	  
		//boost::shared_ptr<GRAPH_DATA> graph_ptr = build_graph();
		boost::shared_ptr<GRAPH_DATA> graph_ptr = build_element_node_graph();
		GRAPH_DATA& graph = *graph_ptr;
		
		
		
		Zoltan *zz = new Zoltan(PE::instance());
		if (zz == NULL)
			throw BadValue(FromHere(),"Zoltan error");

		std::string graph_package = "SCOTCH";
		if (graph_package == "PHG" && PE::instance().size() != 3)
		{
			throw NotImplemented(FromHere(),"PHG graph package needs processor information for each object. It assumes now 3 processors. Run with 3 processors.");
		}
		
			
		zz->Set_Param( "DEBUG_LEVEL", "0");
		zz->Set_Param( "LB_METHOD", "GRAPH");
		zz->Set_Param( "GRAPH_PACKAGE",graph_package);
		zz->Set_Param( "LB_APPROACH", "PARTITION");
		zz->Set_Param( "NUM_GID_ENTRIES", "1 "); 
		zz->Set_Param( "NUM_LID_ENTRIES", "1");
		zz->Set_Param( "RETURN_LISTS", "ALL");
		zz->Set_Param( "GRAPH_SYMMETRIZE","NONE");
		

		zz->Set_Param( "RETURN_LISTS", "ALL");
		zz->Set_Param( "GRAPH_SYMMETRIZE","NONE");

		/* Graph parameters */

		zz->Set_Param( "NUM_GLOBAL_PARTS", "3");
		zz->Set_Param( "CHECK_GRAPH", "2"); 
		zz->Set_Param( "PHG_EDGE_SIZE_THRESHOLD", ".35");  /* 0-remove all, 1-remove none */

		// Query functions 

		zz->Set_Num_Obj_Fn(get_number_of_objects, &graph);
		zz->Set_Obj_List_Fn(get_object_list, &graph);
		zz->Set_Num_Edges_Multi_Fn(get_num_edges_list, &graph);
		zz->Set_Edge_List_Multi_Fn(get_edges_list, &graph);
		
		// partition
		int changes;
		int numGidEntries;
		int numLidEntries;
		int numImport;
		ZOLTAN_ID_PTR importGlobalIds;
		ZOLTAN_ID_PTR importLocalIds;
		int *importProcs;
		int *importToPart;
		int numExport;
		ZOLTAN_ID_PTR exportGlobalIds;
		ZOLTAN_ID_PTR exportLocalIds;
		int *exportProcs;
		int *exportToPart;

		
		
		PE::instance().barrier();
		CFinfo << "before partitioning\n";
		CFinfo << "-------------------" << CFendl;

		std::vector<int> parts (graph.globalID.size());

		for (Uint i=0; i < parts.size(); i++){
			parts[i] = PE::instance().rank();
		}
		
		if(PE::instance().rank()==2)
		{
			CFinfo.setFilterRankZero(false);
			for (Uint i=0; i<graph.globalID.size(); ++i)
			CFinfo << graph.globalID[i] << CFendl;
			CFinfo.setFilterRankZero(true);
		}
		PE::instance().barrier();
		
		
		showGraphPartitions(graph,parts);
		
		
		int rc = zz->LB_Partition(changes, numGidEntries, numLidEntries,
			numImport, importGlobalIds, importLocalIds, importProcs, importToPart,
			numExport, exportGlobalIds, exportLocalIds, exportProcs, exportToPart);

		if (rc != (int)ZOLTAN_OK)
		{
			CFinfo << "Partitioning failed on process " << PE::instance().rank() << CFendl;
		}
		
		PE::instance().barrier();
		CFinfo << "after partitioning\n";
		CFinfo << "------------------" << CFendl;
		
		
		for (int i=0; i < numExport; i++){
			parts[exportLocalIds[i]] = exportToPart[i];
		}
			
		showGraphPartitions(graph,parts);
			
		Zoltan::LB_Free_Part(&importGlobalIds, &importLocalIds, &importProcs, &importToPart);
		Zoltan::LB_Free_Part(&exportGlobalIds, &exportLocalIds, &exportProcs, &exportToPart);  
		
		delete zz;
  }
	
}


//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( zoltan_quadtriag_mesh)
{
 	CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
	meshreader->configure_property("Read Boundaries",false);

	// the file to read from
	boost::filesystem::path fp_in ("quadtriag.neu");

	// the mesh to store in
	CMesh::Ptr mesh_ptr = meshreader->create_mesh_from(fp_in);
  CMesh& mesh = *mesh_ptr;

	// Zoltan
  Zoltan *zz = new Zoltan(PE::instance());
  if (zz == NULL)
    throw BadValue(FromHere(),"Zoltan error");

	
	std::string graph_package = "SCOTCH";
	if (graph_package == "PHG" && PE::instance().size() != 2)
	{
		throw NotImplemented(FromHere(),"PHG graph package needs processor information for each object. It assumes now 2 processors. Run with 2 processors.");
	}
	
  zz->Set_Param( "DEBUG_LEVEL", "1");
  zz->Set_Param( "LB_METHOD", "GRAPH");
  zz->Set_Param( "GRAPH_PACKAGE",graph_package);
  zz->Set_Param( "LB_APPROACH", "PARTITION");
  zz->Set_Param( "NUM_GID_ENTRIES", "1"); 
  zz->Set_Param( "NUM_LID_ENTRIES", "2");
  zz->Set_Param( "RETURN_LISTS", "ALL");
  zz->Set_Param( "GRAPH_SYMMETRIZE","NONE");

  // Graph parameters
  //zz->Set_Param( "NUM_GLOBAL_PARTS", "2");
  zz->Set_Param( "CHECK_GRAPH", "2"); 
  //zz->Set_Param( "PHG_EDGE_SIZE_THRESHOLD", ".35");  /* 0-remove all, 1-remove none */

	// Query functions 
  zz->Set_Num_Obj_Fn(get_number_of_objects_mesh, &mesh);
  zz->Set_Obj_List_Fn(get_object_list_mesh, &mesh);
  zz->Set_Num_Edges_Multi_Fn(get_num_edges_list_mesh, &mesh);
  zz->Set_Edge_List_Multi_Fn(get_edges_list_mesh, &mesh);

  // partition
  int changes;
  int numGidEntries;
  int numLidEntries;
  int numImport;
  ZOLTAN_ID_PTR importGlobalIds;
  ZOLTAN_ID_PTR importLocalIds;
  int *importProcs;
  int *importToPart;
  int numExport;
  ZOLTAN_ID_PTR exportGlobalIds;
  ZOLTAN_ID_PTR exportLocalIds;
  int *exportProcs;
  int *exportToPart;
	
	
	Component::Ptr partition_info = mesh.create_component_type<Component>("temporary_partition_info");
	partition_info->properties()["node_start_idx"]=Uint(0);
	partition_info->properties()["elem_start_idx"]=mesh.property("nb_nodes").value<Uint>();
	Uint node_start_idx = mesh.get_child("temporary_partition_info")->property("node_start_idx").value<Uint>();
	Uint elem_start_idx = mesh.get_child("temporary_partition_info")->property("elem_start_idx").value<Uint>();


	BOOST_CHECK(true);

  int rc = zz->LB_Partition(changes, numGidEntries, numLidEntries,
    numImport, importGlobalIds, importLocalIds, importProcs, importToPart,
    numExport, exportGlobalIds, exportLocalIds, exportProcs, exportToPart);

	BOOST_CHECK(true);

  if (rc != (int)ZOLTAN_OK)
  {
    CFinfo << "Partitioning failed on process " << PE::instance().rank() << CFendl;
  }
	
	
	
/*
	PE::instance().barrier();
  CFinfo << "before partitioning\n";
  CFinfo << "-------------------" << CFendl;
	
  int ierr;
  std::vector<int> parts (get_number_of_objects_mesh(&mesh,&ierr));
	
  for (Uint i=0; i < parts.size(); i++){
    parts[i] = PE::instance().rank();
  }
	
	showMeshPartitions(mesh,parts);

	
  PE::instance().barrier();
  CFinfo << "after partitioning\n";
  CFinfo << "------------------" << CFendl;

	PE_SERIALIZE
	( 
	 {
		 CFinfo << "proc #"<<proc<<CFendl;
		 CFinfo << "-------"<<CFendl;
		 CFinfo << "numExport - numImport = " << numExport - numImport << CFendl;
		 for (int i=0; i < numImport; i++)
		 {
			 std::string global_id = (importGlobalIds[i]<16) ? "node " + to_str(importGlobalIds[i]) : "elem " + to_str(importGlobalIds[i]-16);
			 CFinfo << "importGlobalIds["<<i<<"] = "<<global_id<<CFendl;
			 CFinfo << "local id on proc#"<<importProcs[i]<<" = "<<importLocalIds[i]<<CFendl;

			 CFinfo << CFendl;
		 }
		 
	 }
	)
*/
	
	BOOST_CHECK(true);
/*
	// find import if export is known.
  int& num_known = numExport;
  ZOLTAN_ID_PTR known_global_ids = exportGlobalIds;
  ZOLTAN_ID_PTR known_local_ids  = exportLocalIds;
	int* known_procs = exportProcs;
  int* known_to_part = exportToPart;
		
	int& num_found = numImport;
	ZOLTAN_ID_PTR found_global_ids;// = importGlobalIds;
  ZOLTAN_ID_PTR found_local_ids;//  = importLocalIds;
	int* found_procs;// = importProcs;
  int* found_to_part;// = importToPart;
	
	rc = zz->Invert_Lists ( num_known, known_global_ids, known_local_ids, known_procs, known_to_part, 
													num_found, found_global_ids, found_local_ids, found_procs, found_to_part); 
	
	
	BOOST_CHECK(true);

	*/
	
	PE::instance().barrier();

	//PE_SERIALIZE
	//(
  CFinfo.setFilterRankZero(false);
	
	if (PE::instance().rank() == 1)
	{
	  CFinfo << CFendl;
 	 //CFinfo << "proc " << proc << CFendl;
 	 //CFinfo << "------"<<CFendl;
 		for (int i=0; i < numExport; i++)
 		{
 			if (exportGlobalIds[IDX + numGidEntries*i] < elem_start_idx)
 				CFinfo << "export node " << exportGlobalIds[IDX + numGidEntries*i] - node_start_idx << CFendl;
 			else
 				CFinfo << "export elem " << exportGlobalIds[IDX + numGidEntries*i] - elem_start_idx << CFendl;
 			CFinfo <<   "  component #"<< exportLocalIds[COMP + numLidEntries*i]<<CFendl;
 			CFinfo <<   "  local idx " << exportLocalIds[IDX + numLidEntries*i]<<CFendl;
 			CFinfo <<   "  to proc "   << exportProcs[i] << CFendl;
 			CFinfo <<   "  to part "   << exportToPart[i] << CFendl;
 		}

 		//CFLogVar(numImport);
 		for (int i=0; i < numImport; i++)
 		{
 			if (importGlobalIds[IDX + numGidEntries*i] < elem_start_idx)
 				CFinfo << "import node " << importGlobalIds[IDX + numGidEntries*i] - node_start_idx << CFendl;
 			else
 				CFinfo << "import elem " << importGlobalIds[IDX + numGidEntries*i] - elem_start_idx << CFendl;
 			CFinfo <<   "  component #"<< importLocalIds[COMP + numLidEntries*i]<<CFendl;
 			CFinfo <<   "  local idx " << importLocalIds[IDX + numLidEntries*i]<<CFendl;
 			CFinfo <<   "  from proc " << importProcs[i] << CFendl;
 			CFinfo <<   "  to part "   << importToPart[i] << CFendl;

 			//		Uint loc_id = exportLocalIDs[i] + (obj == NODE ? 0 ; 
 			//    parts[exportLocalIds[i]] = exportToPart[i];
 		}
 	 
	}
  CFinfo.setFilterRankZero(true);

  //showMeshPartitions(mesh,parts);
  	BOOST_CHECK(true);


		PE::instance().barrier();
  
  CFinfo << "before migration\n";
  CFinfo << "------------------" << CFendl;
	
	zz->Set_Obj_Size_Multi_Fn ( get_message_sizes , &mesh );
  zz->Set_Pack_Obj_Multi_Fn ( pack_object_messages , &mesh );
  zz->Set_Unpack_Obj_Multi_Fn ( unpack_object_messages , &mesh );
  zz->Set_Mid_Migrate_PP_Fn ( mid_migrate , &mesh );
	
	
	BOOST_CHECK(true);
	
	// Remove ghost nodes. WARNING: this invalidates the elements and localID references!!!
  //rm_ghost_nodes(mesh);
	
	rc = zz->Migrate( numImport, importGlobalIds, importLocalIds, importProcs, importToPart,
										numExport, exportGlobalIds, exportLocalIds, exportProcs, exportToPart);
	
	BOOST_CHECK(true);
	
	// Remove ghost nodes. WARNING: this invalidates the elements and localID references!!!
  rm_ghost_nodes(mesh);
	
	BOOST_CHECK(true);
	
  
	// export nodes
	// 1) copy nodes to buffer
	// 2) copy global node indices to buffer
	// 3) copy global element conn table to buffer

	// 4) remove through buffer and flush.
	// 5) 
	
  CFinfo << "after migration\n";
  CFinfo << "------------------" << CFendl;

	mesh.remove_component("temporary_partition_info");
	partition_info.reset();

  Zoltan::LB_Free_Part(&importGlobalIds, &importLocalIds, &importProcs, &importToPart);
  Zoltan::LB_Free_Part(&exportGlobalIds, &exportLocalIds, &exportProcs, &exportToPart);  

  delete zz;
	BOOST_CHECK(true);
	
	
	PE_SERIALIZE(
	BOOST_FOREACH(const CArray& coordinates, recursive_filtered_range_typed<CArray>(mesh,IsComponentTag("coordinates")))
  {
    const CList<Uint>& global_node_indices = get_tagged_component_typed< CList<Uint> >(coordinates,"global_node_indices");    
    BOOST_FOREACH(const Uint idx, global_node_indices.array())
    {
      CFinfo << "#" << proc << "   " << idx << CFendl;
    }
  }   
  )
  

} 

////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE( finalize_mpi )
{
	PE::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

