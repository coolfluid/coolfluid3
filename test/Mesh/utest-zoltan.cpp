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
#include <boost/tuple/tuple.hpp>
// coolfluid
#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/Foreach.hpp"

#include "Common/StreamHelpers.hpp"
#include "Common/String/Conversion.hpp"

#include "Math/MatrixTypes.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/ConnectivityData.hpp"
#include "Mesh/CDynTable.hpp"
#include "Mesh/CMeshPartitioner.hpp"
#include "Mesh/Zoltan/LibZoltan.hpp"

#define PE_SERIALIZE(func)                                \
{                                                          \
 PE::instance().barrier();                               \
 CFinfo.setFilterRankZero(false);                        \
  for (Uint proc=0; proc<PE::instance().size(); ++proc)   \
  {                                                       \
    if (proc == PE::instance().rank())                    \
    {                                                     \
      func                                                \
    }                                                     \
    PE::instance().barrier();                             \
  }                                                       \
 CFinfo.setFilterRankZero(true);                         \
 PE::instance().barrier();                               \
}
        
using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace CF::Common::String;
using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

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
	
	enum ObjectIndex { IDX=0, COMP=1 };

  static int get_number_of_objects_mesh(void *data, int *ierr)
  {
    *ierr = ZOLTAN_OK;
    CMesh& mesh = *(CMesh *)data;
    // Uint nb_nodes = find_component<CRegion>(mesh).recursive_nodes_count();
    
    Uint nb_nodes = 0;
    BOOST_FOREACH(const CList<bool>& is_ghost, find_components_recursively_with_tag<CList<bool> >(mesh,"is_ghost"))
    {
      BOOST_FOREACH(const bool is_node_ghost, is_ghost.array())
      {
        if (!is_node_ghost)
          ++nb_nodes;
      }
    }
    
    
    //CFLogVar(nb_nodes);
    Uint nb_elems = find_component<CRegion>(mesh).recursive_elements_count();
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
		
		PE_SERIALIZE(
		CFinfo << "++++++++++++++++++++++++++++++++++ ["<<proc<<"] begin get_object_list_mesh"<<CFendl;
	  )
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
		
		PE_SERIALIZE(
    BOOST_FOREACH(const CList<Uint>& global_node_indices, find_components_recursively_with_tag<CList<Uint> >(mesh,"global_node_indices"))
    {
			//CFinfo << "node comp #"<<component_idx<< " path = " << global_node_indices.get_parent()->full_path().string() << CFendl;

      const CList<bool>& is_ghost = *global_node_indices.get_parent()->get_child<CList<bool> >("is_ghost");
      
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
          CFinfo << "++++++++++ ["<<proc<<"] add node " << glb_idx[IDX] << " at location " << component_idx << " ("<<idx<<")"<<CFendl;
          ++zoltan_idx;
        }
				++idx;
      }
			++component_idx;
    }
    )
    
		// Loop over the elements of the mesh
		// -------------------------------
    
    BOOST_FOREACH(const CList<Uint>& global_element_indices, find_components_recursively_with_tag<CList<Uint> >(mesh,"global_element_indices"))
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
		
		PE_SERIALIZE(
		CFinfo << "++++++++++++++++++++++++++++++++++ ["<<proc<<"] end get_object_list_mesh"<<CFendl;
)
    return;
  }
  
  static void get_num_edges_list_mesh(void *data, int sizeGID, int sizeLID,
                                 int num_obj,
                                 ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                                 int *numEdges, int *ierr)
  {
    
    PE_SERIALIZE(
		CFinfo << "++++++++++++++++++++++++++++++++++ ["<<proc<<"] begin get_num_edges_list_mesh " << CFendl;
    );		
		
    CMesh& mesh = *(CMesh *)data;
    
    *ierr = ZOLTAN_OK;
    
    int error;
    Uint tot_nb_objects =  get_number_of_objects_mesh(data, &error);
  
    Uint zoltan_idx = 0;
    BOOST_FOREACH(const CDynTable<Uint>& node_to_glb_elm, find_components_recursively_with_tag<CDynTable<Uint> >(mesh,"glb_elem_connectivity"))
    {
      if (zoltan_idx >= tot_nb_objects)
			{
				//throw BadValue(FromHere(),"zoltan_idx exceeds tot_nb_objects");
				*ierr = ZOLTAN_FATAL;
				return;
			}
      
      const CList<bool>& is_ghost = *node_to_glb_elm.get_parent()->get_child<CList<bool> >("is_ghost");
      for (Uint i=0; i<node_to_glb_elm.size(); ++i)
      {
        if (!is_ghost[i])
        {
          CDynTable<Uint>::ConstRow glb_elms = node_to_glb_elm[i];
          numEdges[zoltan_idx]=glb_elms.size();
          zoltan_idx++;
        }
      }
    }
        
    BOOST_FOREACH(const CElements& elements, find_components_recursively<CElements>(find_component<CRegion>(mesh)))
    {
      const CTable<Uint>& conn_table = elements.connectivity_table();
      BOOST_FOREACH(CTable<Uint>::ConstRow local_nodes, conn_table.array())
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
    
    PE_SERIALIZE(
		CFinfo << "++++++++++++++++++++++++++++++++++ ["<<proc<<"] end get_num_edges_list_mesh " << CFendl;
    );		
		
    
    *ierr = ZOLTAN_OK;
    return;
  }

	
	
	
	
	static Uint hash_proc_nodes(CMesh& mesh, Uint glb_idx)
	{
	  Real np = PE::instance().size();
    Real nb_nodes = mesh.property("nb_nodes").value<Uint>();
    Real part_size = std::floor(nb_nodes/np);
    
    return static_cast<Uint>(std::min(np-1.,std::floor(static_cast<Real>(glb_idx)/part_size)));
	}
	
	static Uint hash_proc_elems(CMesh& mesh, Uint glb_idx)
	{
    Real np = PE::instance().size();
    Real nb_cells = mesh.property("nb_cells").value<Uint>();
    Real part_size = std::floor(nb_cells/np);
    
    return static_cast<Uint>(std::min(np-1.,std::floor(static_cast<Real>(glb_idx)/part_size)));
	}
	
	
	
	
	
	
	
	
	
	
	
  static void get_edges_list_mesh(void *data, int sizeGID, int sizeLID,
          int num_obj, ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
          int *num_edges,
          ZOLTAN_ID_PTR nborGID, int *nborProc,
          int wgt_dim, float *ewgts, int *ierr)
  {
    CMesh& mesh = *(CMesh *)data;
    *ierr = ZOLTAN_OK;

		PE_SERIALIZE(
		CFinfo << "++++++++++++++++++++++++++++++++++ ["<<proc<<"] begin get_edges_list_mesh " << CFendl;
    );		


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

		BOOST_FOREACH(const CDynTable<Uint>& node_to_glb_elm, find_components_recursively_with_tag<CDynTable<Uint> >(mesh,"glb_elem_connectivity"))
    {
      const CList<bool>& is_ghost = *node_to_glb_elm.get_parent()->get_child<CList<bool> >("is_ghost");
      for (Uint i=0; i<node_to_glb_elm.size(); ++i)
      {
        if (!is_ghost[i])
        {
          CDynTable<Uint>::ConstRow glb_elms = node_to_glb_elm[i];
          for (Uint j=0; j<glb_elms.size(); ++j)
          {
						num_edges_from_nodes++;
						*nbor_glb_idx++ = elem_start_idx + glb_elms[j];
						//CFinfo << "   " << i << " --> " << elem_start_idx + glb_elms[j] << CFendl;
            *nbor_proc++ = hash_proc_elems(mesh,glb_elms[j]);
          }
        }
      }
    }

    BOOST_FOREACH(const CElements& elements, find_components_recursively<CElements>(find_component<CRegion>(mesh)))
    {
      const CTable<Real>& coordinates = elements.coordinates();
      const CList<Uint>& glb_node_idx = find_component_with_tag< CList<Uint> > (coordinates,"global_node_indices");
			//const CList<Uint>& glb_elm_idx = find_component_with_tag< CList<Uint> > (elements,"global_element_indices");

      const CTable<Uint>& conn_table = elements.connectivity_table();
			Uint loc_elm_idx = 0;
      BOOST_FOREACH(CTable<Uint>::ConstRow local_nodes, conn_table.array())
      {        
        BOOST_FOREACH(const Uint loc_node, local_nodes)
        {					
					num_edges_from_elems++;
					*nbor_glb_idx++ = node_start_idx + glb_node_idx[loc_node];
					//CFinfo << "   " << glb_elm_idx[loc_elm_idx] << " --> " << node_start_idx + glb_node_idx[loc_node] << CFendl;

          *nbor_proc++ = hash_proc_nodes(mesh,glb_node_idx[loc_node]);
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
    
    PE_SERIALIZE(
		CFinfo << "++++++++++++++++++++++++++++++++++ ["<<proc<<"] end get_edges_list_mesh " << CFendl;
    );		
		
  }
    
  
	/* Draw the partition assignments of the objects */

  static void showMeshPartitions(CMesh& mesh, std::vector<int>& parts)
  {
    Uint nb_nodes = mesh.property("nb_nodes").value<Uint>();
    Uint nb_cells = mesh.property("nb_cells").value<Uint>();
    Uint tot_num_objs = nb_nodes + nb_cells;
    
    std::vector<int> part_assign_on_this_proc(tot_num_objs);
    std::vector<int> part_assign(tot_num_objs);
    
    CList<Uint>& global_id = *mesh.get_child<CList<Uint> >("global_graph_id");
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
	
	static void get_nodes_sizes(void *data, int gidSize, int lidSize, int num_ids,
																ZOLTAN_ID_PTR globalIDs, ZOLTAN_ID_PTR localIDs, int *sizes, int *ierr)
{ 
  PE_SERIALIZE(
		CFinfo << "++++++++++++++++++++++++++++++++++ ["<<proc<<"] begin get_nodes_sizes " << CFendl;
    );		
    
		CMesh& mesh = *(CMesh *)data;
		*ierr = ZOLTAN_OK;
		
		std::vector<Component::ConstPtr> components;
		BOOST_FOREACH(const CTable<Real>& coordinates, 
									find_components_recursively_with_tag<CTable<Real> >(mesh,"coordinates"))
			components.push_back(coordinates.get());
    BOOST_FOREACH(const CElements& elements, find_components_recursively<CElements>(mesh))
			components.push_back(elements.get());
		
		std::vector<CDynTable<Uint>::ConstPtr> list_of_node_to_glb_elm;
		BOOST_FOREACH(const CDynTable<Uint>& node_to_glb_elm, find_components_recursively_with_tag<CDynTable<Uint> >(mesh,"glb_elem_connectivity"))
      list_of_node_to_glb_elm.push_back(node_to_glb_elm.as_type<CDynTable<Uint> >());
		
		//Uint node_start_idx = mesh.get_child("temporary_partition_info")->property("node_start_idx").value<Uint>();
		Uint elem_start_idx = mesh.get_child("temporary_partition_info")->property("elem_start_idx").value<Uint>();

		for (int i=0; i < num_ids; i++) 
		{
			int* loc_id = (int*)(localIDs+i*lidSize);
			int* glb_id = (int*)(globalIDs+i*gidSize);

			if (glb_id[IDX] < (int) elem_start_idx)
			{
				sizes[i] = sizeof(Uint) // component index
				         + sizeof(Real) * components[loc_id[COMP]]->as_type<CTable<Real> >()->row_size() // coordinates
                 + sizeof(Uint) * (1+list_of_node_to_glb_elm[loc_id[COMP]]->row_size(loc_id[IDX])); // global element indices that need this node
			}
			else
			{
				sizes[i] = 0;
			}

		}
		
		PE_SERIALIZE(
  		CFinfo << "++++++++++++++++++++++++++++++++++ ["<<proc<<"] end get_nodes_sizes " << CFendl;
      );
	}
	
	
	static void pack_nodes_messages(void *data, int gidSize, int lidSize, int num_ids,
																	 ZOLTAN_ID_PTR globalIDs, ZOLTAN_ID_PTR localIDs, int *dests, int *sizes, int *idx, char *buf, int *ierr)
	{
		
		PE_SERIALIZE(
		CFinfo << "++++++++++++++++++++++++++++++++++ ["<<proc<<"] begin pack_nodes_messages " << CFendl;
    );		
    CMesh& mesh = *(CMesh *)data;
		*ierr = ZOLTAN_OK;
		
    Uint* component_number;
		Real* coord_row_buf;
    Uint* glb_elm_idx_buf;
    
		std::vector<Component::Ptr> components;
		BOOST_FOREACH(CTable<Real>& coordinates, 
									find_components_recursively_with_tag<CTable<Real> >(mesh,"coordinates"))
			components.push_back(coordinates.get());
			
		std::vector<CDynTable<Uint>::ConstPtr> list_of_node_to_glb_elm;
		BOOST_FOREACH(const CDynTable<Uint>& node_to_glb_elm, find_components_recursively_with_tag<CDynTable<Uint> >(mesh,"glb_elem_connectivity"))
      list_of_node_to_glb_elm.push_back(node_to_glb_elm.as_type<CDynTable<Uint> >());
		
		std::vector<CList<bool>::Ptr> list_of_is_ghost;
		BOOST_FOREACH(CList<bool>& is_ghost, find_components_recursively_with_tag<CList<bool> >(mesh,"is_ghost"))
      list_of_is_ghost.push_back(is_ghost.as_type<CList<bool> >());		
		
		//Uint node_start_idx = mesh.get_child("temporary_partition_info")->property("node_start_idx").value<Uint>();
		Uint elem_start_idx = mesh.get_child("temporary_partition_info")->property("elem_start_idx").value<Uint>();

    PE_SERIALIZE(
      CFinfo << "+++++++ ["<<proc<<"] assembled components" << CFendl;
    )

    PE_SERIALIZE(
  	for (int id=0; id < num_ids; id++) 
		{
			int* loc_id = (int*)(localIDs+id*lidSize);
			int* glb_id = (int*)(globalIDs+id*gidSize);
		
			if (glb_id[IDX] < (int) elem_start_idx)
			{	
			  CFinfo << "+++++++ ["<<proc<<"] packing node " << glb_id[IDX] << " :    " <<  CFflush;
        
        component_number = (Uint *)(buf + idx[id]);
        *component_number++ = loc_id[COMP];
        CFinfo << " comp = " << loc_id[COMP] << "      coord_idx = " << CFflush;
        CFinfo << loc_id[IDX] << "/" << components[loc_id[COMP]]->as_type<CTable<Real> >()->size() << "    coords = " << CFflush;
				coord_row_buf = (Real *)(component_number);
				BOOST_FOREACH(const Real& coord, components[loc_id[COMP]]->as_type<CTable<Real> >()->array()[loc_id[IDX]])
				{
				  *coord_row_buf++ = coord;
          CFinfo << coord << "  " ;
				}
        CFinfo << CFendl;
			
        glb_elm_idx_buf = (Uint *)(coord_row_buf);
        *glb_elm_idx_buf++ = list_of_node_to_glb_elm[loc_id[COMP]]->row_size(loc_id[IDX]);
				BOOST_FOREACH(const Uint& glb_elem_idx, list_of_node_to_glb_elm[loc_id[COMP]]->array()[loc_id[IDX]])
					*glb_elm_idx_buf++ = glb_elem_idx;

			  // mark this node as ghost
        list_of_is_ghost[loc_id[COMP]]->array()[loc_id[IDX]] = true;
			}
		}
		)
		PE_SERIALIZE(
		CFinfo << "++++++++++++++++++++++++++++++++++ ["<<proc<<"] end pack_nodes_messages " << CFendl;
    );
	}


	static void unpack_nodes_messages(void *data, int gidSize, int num_ids,
																		 ZOLTAN_ID_PTR globalIDs, int *sizes, int *idx, char *buf, int *ierr)
	{		
	  CFinfo << "++++++++++++++++++++++++++++++++++ unpack_nodes_messages"<<CFendl;
    
		CMesh& mesh = *(CMesh *)data;
		*ierr = ZOLTAN_OK;

		std::vector< boost::shared_ptr<CTable<Real>::Buffer> > coordinates_buffer;
    std::vector< boost::shared_ptr<CList<bool>::Buffer> > is_ghost_buffer;
    std::vector< boost::shared_ptr<CList<Uint>::Buffer> > glb_node_indices_buffer;
    std::vector< boost::shared_ptr<CDynTable<Uint>::Buffer> > node_to_glb_elms_buffer;
		
		
		BOOST_FOREACH(CTable<Real>& coords, 
									find_components_recursively_with_tag<CTable<Real> >(mesh,"coordinates"))
		{
		  coordinates_buffer.push_back( boost::shared_ptr<CTable<Real>::Buffer> (new CTable<Real>::Buffer(coords.as_type<CTable<Real> >()->create_buffer())));
			is_ghost_buffer.push_back( boost::shared_ptr<CList<bool>::Buffer> (new CList<bool>::Buffer(find_component_with_tag<CList<bool> >(coords,"is_ghost").create_buffer())));
			glb_node_indices_buffer.push_back( boost::shared_ptr<CList<Uint>::Buffer> (new CList<Uint>::Buffer(find_component_with_tag<CList<Uint> >(coords,"global_node_indices").create_buffer())));
			node_to_glb_elms_buffer.push_back( boost::shared_ptr<CDynTable<Uint>::Buffer> (new CDynTable<Uint>::Buffer(find_component_with_tag<CDynTable<Uint> >(coords,"glb_elem_connectivity").create_buffer())));
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
	
	
	static void rm_ghost_nodes(CMesh& mesh)
  {
    BOOST_FOREACH(CTable<Real>& coordinates, find_components_recursively_with_tag<CTable<Real> >(mesh,"coordinates"))
    {
      CList<bool>& is_ghost = find_component_with_tag< CList<bool> >(coordinates,"is_ghost");
      CList<Uint>& global_node_indices = find_component_with_tag< CList<Uint> >(coordinates,"global_node_indices");
      CDynTable<Uint>& glb_elem_connectivity = find_component_with_name< CDynTable<Uint> >(coordinates,"glb_elem_connectivity");

      CFLogVar(coordinates.size());
      CFLogVar(is_ghost.size());
      CFLogVar(global_node_indices.size());
      CFLogVar(glb_elem_connectivity.size());


      CList<bool>::Buffer buffer_is_ghost = is_ghost.create_buffer();
      CList<Uint>::Buffer buffer_global_node_indices = global_node_indices.create_buffer();
      CDynTable<Uint>::Buffer buffer_glb_elem_connectivity = glb_elem_connectivity.create_buffer();
      
      CTable<Real>::Buffer buffer_coordinates = coordinates.create_buffer();
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
    
    BOOST_FOREACH(const CTable<Real>& coordinates, find_components_recursively_with_tag<CTable<Real> >(mesh,"coordinates"))
    {
      const CList<bool>& is_ghost = find_component_with_tag< CList<bool> >(coordinates,"is_ghost");
      const CList<Uint>& global_node_indices = find_component_with_tag< CList<Uint> >(coordinates,"global_node_indices");
      const CDynTable<Uint>& glb_elem_connectivity = find_component_with_name< CDynTable<Uint> >(coordinates,"glb_elem_connectivity");
      
      
      CFLogVar(coordinates.size());
      CFLogVar(is_ghost.size());
      CFLogVar(global_node_indices.size());
      CFLogVar(glb_elem_connectivity.size());  
    }    
    
  }


	void give_elems_global_node_numbers(CMesh& mesh)
  {
    CFinfo << "++++++++++++++++++++++++++++++++++++++++++++ give_elems_global_node_numbers" << CFendl;
    BOOST_FOREACH(CElements& elements, find_components_recursively<CElements>(mesh))
    {
      CTable<Uint>& conn_table = elements.connectivity_table();
      const CTable<Real>& coordinates = elements.coordinates();
      const CList<Uint>& global_node_indices = find_component_with_tag< CList<Uint> >(coordinates,"global_node_indices");

      BOOST_FOREACH ( CTable<Uint>::Row nodes, conn_table.array() )
      {
        BOOST_FOREACH ( Uint& node, nodes )
        {
          node = global_node_indices[node];
        }
      }
    }    
  }
  
  void give_elems_local_node_numbers(CMesh& mesh)
  {
    CFinfo << "++++++++++++++++++++++++++++++++++++++++++++ give_elems_local_node_numbers" << CFendl;
    std::map<Uint,Uint> glb_to_loc;
    
    BOOST_FOREACH(const CTable<Real>& coordinates, find_components_recursively_with_tag<CTable<Real> >(mesh,"coordinates"))
    {
      const CList<Uint>& global_node_indices = find_component_with_tag< CList<Uint> >(coordinates,"global_node_indices");
      for (Uint i=0; i<coordinates.size(); ++i)
      {
        glb_to_loc[global_node_indices[i]]=i;
      }
    }
    
    BOOST_FOREACH(CElements& elements, find_components_recursively<CElements>(mesh))
    {
      CTable<Uint>& conn_table = elements.connectivity_table();

      BOOST_FOREACH ( CTable<Uint>::Row nodes, conn_table.array() )
      {
        BOOST_FOREACH ( Uint& node, nodes )
        {
          node = glb_to_loc[node];
        }
      }
    }    
  }
	
  std::set<Uint> get_ghost_nodes_to_import(CMesh& mesh)
  {
    CFinfo << "++++++++++++++++++++++++++++++++++++++++++++ get_ghost_nodes_to_import" << CFendl;
    
    std::set<Uint> nodes_needed_by_elems;
    BOOST_FOREACH(CElements& elements, find_components_recursively<CElements>(mesh))
    {
      CTable<Uint>& conn_table = elements.connectivity_table();
      
      BOOST_FOREACH ( CTable<Uint>::Row nodes, conn_table.array() )
      {
        BOOST_FOREACH ( Uint& node, nodes )
        {
          nodes_needed_by_elems.insert(node);
        }
      }
    }
    BOOST_FOREACH(const CTable<Real>& coordinates, find_components_recursively_with_tag<CTable<Real> >(mesh,"coordinates"))
    {
      const CList<Uint>& global_node_indices = find_component_with_tag< CList<Uint> >(coordinates,"global_node_indices");
      
      BOOST_FOREACH (const Uint node, global_node_indices.array() )
      {
        std::set<Uint>::iterator it = nodes_needed_by_elems.find(node);
        std::set<Uint>::iterator not_found = nodes_needed_by_elems.end();
        
        if (it != not_found)
        {
          nodes_needed_by_elems.erase(it);
        }
      }
    }
    
    BOOST_FOREACH (const Uint node, nodes_needed_by_elems)
    {
      CFinfo << "ghost node: " << node << CFendl;
    }
    
    return nodes_needed_by_elems;
  }
  
  static void get_elems_sizes(void *data, int gidSize, int lidSize, int num_ids,
																ZOLTAN_ID_PTR globalIDs, ZOLTAN_ID_PTR localIDs, int *sizes, int *ierr)
	{
	  CFinfo << "++++++++++++++++++++++++++++++++++ get_elems_sizes"<<CFendl;
    
		CMesh& mesh = *(CMesh *)data;
		*ierr = ZOLTAN_OK;
		
		std::vector<Component::Ptr> components;
		BOOST_FOREACH(CTable<Real>& coordinates, 
									find_components_recursively_with_tag<CTable<Real> >(mesh,"coordinates"))
			components.push_back(coordinates.get());
    BOOST_FOREACH(CElements& elements, find_components_recursively<CElements>(mesh))
      components.push_back(elements.get());
		
				
		//Uint node_start_idx = mesh.get_child("temporary_partition_info")->property("node_start_idx").value<Uint>();
		Uint elem_start_idx = mesh.get_child("temporary_partition_info")->property("elem_start_idx").value<Uint>();

		for (int i=0; i < num_ids; i++) 
		{
			int* loc_id = (int*)(localIDs+i*lidSize);
			int* glb_id = (int*)(globalIDs+i*gidSize);

			if (glb_id[IDX] >= (int) elem_start_idx)
			{
				sizes[i] = sizeof(Uint) // component index
                 + sizeof(Uint) * components[loc_id[COMP]]->as_type<CElements>()->connectivity_table().row_size(); // nodes
			}
			else
			{
				sizes[i] = 0;
			}

		}
	}
	
	static void pack_elems_messages(void *data, int gidSize, int lidSize, int num_ids,
																	 ZOLTAN_ID_PTR globalIDs, ZOLTAN_ID_PTR localIDs, int *dests, int *sizes, int *idx, char *buf, int *ierr)
	{
		
    CFinfo << "++++++++++++++++++++++++++++++++++ begin pack_elems_messages"<<CFendl;
		CMesh& mesh = *(CMesh *)data;
		*ierr = ZOLTAN_OK;
		
    Uint* component_number;
		Uint* nodes_buf;
    
    std::vector<boost::shared_ptr<CTable<Uint>::Buffer> > elem_buffer;
		std::vector<Component::Ptr> components;
		BOOST_FOREACH(CTable<Real>& coordinates, 
									find_components_recursively_with_tag<CTable<Real> >(mesh,"coordinates"))
		{
      elem_buffer.push_back(boost::shared_ptr<CTable<Uint>::Buffer>());
		  components.push_back(coordinates.get());
	  }
    BOOST_FOREACH(CElements& elements, find_components_recursively<CElements>(mesh))
    {
      elem_buffer.push_back(boost::shared_ptr<CTable<Uint>::Buffer> ( new CTable<Uint>::Buffer(elements.connectivity_table().create_buffer())));
      components.push_back(elements.get());
    }

		
		
		//Uint node_start_idx = mesh.get_child("temporary_partition_info")->property("node_start_idx").value<Uint>();
		Uint elem_start_idx = mesh.get_child("temporary_partition_info")->property("elem_start_idx").value<Uint>();

  	for (int id=0; id < num_ids; id++) 
		{
			int* loc_id = (int*)(localIDs+id*lidSize);
			int* glb_id = (int*)(globalIDs+id*gidSize);
		
			if (glb_id[IDX] >= (int) elem_start_idx)
			{	
        CFinfo << "+++++++ packing elem " << glb_id[IDX] - elem_start_idx << " : ";
        component_number = (Uint *)(buf + idx[id]);
        *component_number++ = loc_id[COMP];

        nodes_buf = (Uint *)(component_number);

			  BOOST_FOREACH(const Uint node, components[loc_id[COMP]]->as_type<CElements>()->connectivity_table()[loc_id[IDX]])
			  {
          CFinfo << " " << node;
			    *nodes_buf++ = node;
			  }
        CFinfo << CFendl;
			  	
			  //CFinfo << "removing row " << loc_id[IDX] << " from buffer " << loc_id[COMP] << CFendl;
        
        elem_buffer[loc_id[COMP]]->rm_row(loc_id[IDX]);
			  
			}
		}
		
		CFinfo << "++++++++++++++++++++++++++++++++++ end pack_elems_messages"<<CFendl;
    
	}
	
	
	
	static void unpack_elems_messages(void *data, int gidSize, int num_ids,
																		 ZOLTAN_ID_PTR globalIDs, int *sizes, int *idx, char *buf, int *ierr)
	{
		
    CFinfo << "++++++++++++++++++++++++++++++++++ unpack_elems_messages"<<CFendl;
		CMesh& mesh = *(CMesh *)data;
		*ierr = ZOLTAN_OK;
		
    Uint* component_number;
		Uint* nodes_buf;
    Uint comp_idx;
    
    std::vector<boost::shared_ptr<CTable<Uint>::Buffer> > elem_buffer;
		std::vector<Component::Ptr> components;
		BOOST_FOREACH(CTable<Real>& coordinates, 
									find_components_recursively_with_tag<CTable<Real> >(mesh,"coordinates"))
		{
      elem_buffer.push_back(boost::shared_ptr<CTable<Uint>::Buffer>());
		  components.push_back(coordinates.get());
	  }
    BOOST_FOREACH(CElements& elements, find_components_recursively<CElements>(mesh))
    {
      elem_buffer.push_back(boost::shared_ptr<CTable<Uint>::Buffer> ( new CTable<Uint>::Buffer(elements.connectivity_table().create_buffer())));
      components.push_back(elements.get());
    }

		
		
		//Uint node_start_idx = mesh.get_child("temporary_partition_info")->property("node_start_idx").value<Uint>();
		Uint elem_start_idx = mesh.get_child("temporary_partition_info")->property("elem_start_idx").value<Uint>();

  	for (int id=0; id < num_ids; id++) 
		{
			int* glb_id = (int*)(globalIDs+id*gidSize);
		
			if (glb_id[IDX] >= (int) elem_start_idx)
			{	
			  CFinfo << "+++++++ unpacking elem " << glb_id[IDX] - elem_start_idx << " : ";
        
        component_number = (Uint *)(buf + idx[id]);
        comp_idx = *component_number;

        Uint nb_nodes = components[comp_idx]->as_type<CElements>()->connectivity_table().row_size();
        std::vector<Uint> nodes(nb_nodes);
                
        nodes_buf = (Uint *)(++component_number);
        for (Uint i=0; i<nb_nodes; ++i)
        {
          nodes[i] = *nodes_buf++;
          CFinfo << " " << nodes[i];
        }
        CFinfo << CFendl;
			  
        //CFinfo << "adding row to buffer " << comp_idx << CFendl;
        elem_buffer[comp_idx]->add_row(nodes);
			  
			}
		}
	}
	
	static void post_migrate_elems(void *data, int gidSize, int lidSize,
													int numImport, ZOLTAN_ID_PTR importGlobalID, ZOLTAN_ID_PTR importLocalID, int *importProc, int *importPart,
													int numExport, ZOLTAN_ID_PTR exportGlobalID, ZOLTAN_ID_PTR exportLocalID, int *exportProc, int *exportPart,
													int *ierr)
	{
		*ierr = ZOLTAN_OK;
    CFinfo << "++++++++++++++++++++++++++++++++++ post_migrate_elems"<<CFendl;
		CMesh& mesh = *(CMesh *)data;

    // may not rm_ghost-nodes here because then at node_migration, indexes are not valid
    //rm_ghost_nodes(mesh);

    std::set<Uint>::iterator it;
    std::set<Uint>::iterator not_found = m_ghost_nodes.end();

        
    // 1) put in ghost_nodes initially ALL the nodes required by the migrated elements
    BOOST_FOREACH(CElements& elements, find_components_recursively<CElements>(mesh))
    {
      CTable<Uint>& conn_table = elements.connectivity_table();
      
      BOOST_FOREACH ( CTable<Uint>::Row nodes, conn_table.array() )
      {
        BOOST_FOREACH ( Uint& node, nodes )
        {
         m_ghost_nodes.insert(node);
        }
      }
    }
    
    CFinfo << "nodes after step 1 = ";
    BOOST_FOREACH(Uint node, m_ghost_nodes)
      CFinfo << " " << node ;
    CFinfo << CFendl;
    
    // 2) remove from ghost_nodes ALL the nodes that are present in the coordinate tables
    BOOST_FOREACH(const CTable<Real>& coordinates, find_components_recursively_with_tag<CTable<Real> >(mesh,"coordinates"))
    {
      const CList<Uint>& global_node_indices = find_component_with_tag< CList<Uint> >(coordinates,"global_node_indices");
      const CList<bool>& is_ghost = find_component_with_tag< CList<bool> >(coordinates,"is_ghost");      
      for (Uint i=0; i<coordinates.size(); ++i)
      {
        if (!is_ghost[i])
        {
          it = m_ghost_nodes.find(global_node_indices[i]);
          // delete node from ghost_nodes if it is found
          if (it == not_found)
            m_ghost_nodes.insert(global_node_indices[i]);
          else
            m_ghost_nodes.erase(it);
        }
      }
    }
    
    CFinfo << "nodes after step 2 = ";
    BOOST_FOREACH(Uint node, m_ghost_nodes)
      CFinfo << " " << node ;
    CFinfo << CFendl;
    
    // 3) add to ghost_nodes all the nodes that are going to be exported
    Uint elem_start_idx = mesh.get_child("temporary_partition_info")->property("elem_start_idx").value<Uint>();
    std::set<Uint> nodes_to_export;
    for (int id=0; id < numExport; id++) 
		{
			int* glb_id = (int*)(exportGlobalID+id*gidSize);

			if (glb_id[IDX] < (int) elem_start_idx) // if it is a node
			{	
        it = m_ghost_nodes.find(glb_id[IDX]);

        if (it == not_found) // if not found
          m_ghost_nodes.insert(glb_id[IDX]);
        else // if found
          m_ghost_nodes.erase(it);
			}
		}
		
		CFinfo << "nodes after step 3 = ";
    BOOST_FOREACH(Uint node, m_ghost_nodes)
      CFinfo << " " << node ;
    CFinfo << CFendl;
    
		
		// 4) remove from ghost_nodes all the nodes that are going to be imported
		std::set<Uint> nodes_to_import;
    for (int id=0; id < numImport; id++) 
		{
			int* glb_id = (int*)(importGlobalID+id*gidSize);

			if (glb_id[IDX] < (int) elem_start_idx) // if it is a node
			{	
        it = m_ghost_nodes.find(glb_id[IDX]);        
        // delete node from ghost_nodes if it is found
        if (it != not_found) // if found
        {
          m_ghost_nodes.erase(it);
        }
			}
		}
    
    CFinfo << "nodes after step 4 = ";
    BOOST_FOREACH(Uint node, m_ghost_nodes)
      CFinfo << " " << node ;
    CFinfo << CFendl;
    
    
      // BOOST_FOREACH (const Uint ghost_node, m_ghost_nodes)
      // {
      //   CFinfo << "ghost node: " << ghost_node << CFendl;
      // }
    
	}
	
	
	static void get_ghost_nodes_sizes(void *data, int gidSize, int lidSize, int num_ids,
																ZOLTAN_ID_PTR globalIDs, ZOLTAN_ID_PTR localIDs, int *sizes, int *ierr)
	{
	  PE_SERIALIZE(
	  CFinfo << "++++++++++++++++++++++++++++++++++ ["<<proc<<"] begin get_ghost_nodes_sizes"<<CFendl;
    )
		CMesh& mesh = *(CMesh *)data;
		*ierr = ZOLTAN_OK;
		
		std::vector<Component::ConstPtr> components;
		BOOST_FOREACH(const CTable<Real>& coordinates, 
									find_components_recursively_with_tag<CTable<Real> >(mesh,"coordinates"))
			components.push_back(coordinates.get());
    BOOST_FOREACH(const CElements& elements, find_components_recursively<CElements>(mesh))
			components.push_back(elements.get());
		
		std::vector<CDynTable<Uint>::ConstPtr> list_of_node_to_glb_elm;
		BOOST_FOREACH(const CDynTable<Uint>& node_to_glb_elm, find_components_recursively_with_tag<CDynTable<Uint> >(mesh,"glb_elem_connectivity"))
      list_of_node_to_glb_elm.push_back(node_to_glb_elm.as_type<CDynTable<Uint> >());
		
		//Uint node_start_idx = mesh.get_child("temporary_partition_info")->property("node_start_idx").value<Uint>();
		Uint elem_start_idx = mesh.get_child("temporary_partition_info")->property("elem_start_idx").value<Uint>();

		for (int i=0; i < num_ids; i++) 
		{
			int* loc_id = (int*)(localIDs+i*lidSize);
			int* glb_id = (int*)(globalIDs+i*gidSize);

			if (glb_id[IDX] < (int) elem_start_idx)
			{
				sizes[i] = sizeof(Uint) // component index
				         + sizeof(Real) * components[loc_id[COMP]]->as_type<CTable<Real> >()->row_size() // coordinates
                 + sizeof(Uint) * (1+list_of_node_to_glb_elm[loc_id[COMP]]->row_size(loc_id[IDX])); // global element indices that need this node
			}
			else
			{
				sizes[i] = 0;
			}

		}
		
		PE_SERIALIZE(
	  CFinfo << "++++++++++++++++++++++++++++++++++ ["<<proc<<"] end get_ghost_nodes_sizes"<<CFendl;
    )
	  
	}
	
	
	static void pack_ghost_nodes_messages(void *data, int gidSize, int lidSize, int num_ids,
																	 ZOLTAN_ID_PTR globalIDs, ZOLTAN_ID_PTR localIDs, int *dests, int *sizes, int *idx, char *buf, int *ierr)
	{
		
		PE_SERIALIZE(
		CFinfo << "++++++++++++++++++++++++++++++++++ ["<<proc<<"] begin pack_ghost_nodes_messages " << CFendl;
    );		
    CMesh& mesh = *(CMesh *)data;
		*ierr = ZOLTAN_OK;
		
    Uint* component_number;
		Real* coord_row_buf;
    Uint* glb_elm_idx_buf;
    
		std::vector<Component::Ptr> components;
		BOOST_FOREACH(CTable<Real>& coordinates, 
									find_components_recursively_with_tag<CTable<Real> >(mesh,"coordinates"))
			components.push_back(coordinates.get());
			
		std::vector<CDynTable<Uint>::ConstPtr> list_of_node_to_glb_elm;
		BOOST_FOREACH(const CDynTable<Uint>& node_to_glb_elm, find_components_recursively_with_tag<CDynTable<Uint> >(mesh,"glb_elem_connectivity"))
      list_of_node_to_glb_elm.push_back(node_to_glb_elm.as_type<CDynTable<Uint> >());
		
		std::vector<CList<bool>::Ptr> list_of_is_ghost;
		BOOST_FOREACH(CList<bool>& is_ghost, find_components_recursively_with_tag<CList<bool> >(mesh,"is_ghost"))
      list_of_is_ghost.push_back(is_ghost.as_type<CList<bool> >());		
		
		//Uint node_start_idx = mesh.get_child("temporary_partition_info")->property("node_start_idx").value<Uint>();
		Uint elem_start_idx = mesh.get_child("temporary_partition_info")->property("elem_start_idx").value<Uint>();

    PE_SERIALIZE(
      CFinfo << "+++++++ ["<<proc<<"] assembled components" << CFendl;
    )

    PE_SERIALIZE(
  	for (int id=0; id < num_ids; id++) 
		{
			int* loc_id = (int*)(localIDs+id*lidSize);
			int* glb_id = (int*)(globalIDs+id*gidSize);
		
			if (glb_id[IDX] < (int) elem_start_idx)
			{	
			  CFinfo << "+++++++ ["<<proc<<"] packing node " << glb_id[IDX] << " :    " <<  CFflush;
        
        component_number = (Uint *)(buf + idx[id]);
        *component_number++ = loc_id[COMP];
        CFinfo << " comp = " << loc_id[COMP] << "      coord_idx = " << CFflush;
        CFinfo << loc_id[IDX] << "/" << components[loc_id[COMP]]->as_type<CTable<Real> >()->size() << "    coords = " << CFflush;
				coord_row_buf = (Real *)(component_number);
				BOOST_FOREACH(const Real& coord, components[loc_id[COMP]]->as_type<CTable<Real> >()->array()[loc_id[IDX]])
				{
				  *coord_row_buf++ = coord;
          CFinfo << coord << "  " ;
				}
        CFinfo << CFendl;
			
        glb_elm_idx_buf = (Uint *)(coord_row_buf);
        *glb_elm_idx_buf++ = list_of_node_to_glb_elm[loc_id[COMP]]->row_size(loc_id[IDX]);
				BOOST_FOREACH(const Uint& glb_elem_idx, list_of_node_to_glb_elm[loc_id[COMP]]->array()[loc_id[IDX]])
					*glb_elm_idx_buf++ = glb_elem_idx;
			}
		}
		)
		PE_SERIALIZE(
		CFinfo << "++++++++++++++++++++++++++++++++++ ["<<proc<<"] end pack_ghost_nodes_messages " << CFendl;
    );
	}


	static void unpack_ghost_nodes_messages(void *data, int gidSize, int num_ids,
																		 ZOLTAN_ID_PTR globalIDs, int *sizes, int *idx, char *buf, int *ierr)
	{		
	  CFinfo << "++++++++++++++++++++++++++++++++++ unpack_ghost_nodes_messages"<<CFendl;
    
		CMesh& mesh = *(CMesh *)data;
		*ierr = ZOLTAN_OK;

		std::vector< boost::shared_ptr<CTable<Real>::Buffer> > coordinates_buffer;
    std::vector< boost::shared_ptr<CList<bool>::Buffer> > is_ghost_buffer;
    std::vector< boost::shared_ptr<CList<Uint>::Buffer> > glb_node_indices_buffer;
    std::vector< boost::shared_ptr<CDynTable<Uint>::Buffer> > node_to_glb_elms_buffer;
		
		
		BOOST_FOREACH(CTable<Real>& coords, 
									find_components_recursively_with_tag<CTable<Real> >(mesh,"coordinates"))
		{
		  coordinates_buffer.push_back( boost::shared_ptr<CTable<Real>::Buffer> (new CTable<Real>::Buffer(coords.as_type<CTable<Real> >()->create_buffer())));
			is_ghost_buffer.push_back( boost::shared_ptr<CList<bool>::Buffer> (new CList<bool>::Buffer(find_component_with_tag<CList<bool> >(coords,"is_ghost").create_buffer())));
			glb_node_indices_buffer.push_back( boost::shared_ptr<CList<Uint>::Buffer> (new CList<Uint>::Buffer(find_component_with_tag<CList<Uint> >(coords,"global_node_indices").create_buffer())));
			node_to_glb_elms_buffer.push_back( boost::shared_ptr<CDynTable<Uint>::Buffer> (new CDynTable<Uint>::Buffer(find_component_with_tag<CDynTable<Uint> >(coords,"glb_elem_connectivity").create_buffer())));
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
         is_ghost_buffer[comp_idx]->add_row(true);
         
         glb_elm_idx_buf = (Uint *)(coord_row);
         std::vector<Uint> elems(*glb_elm_idx_buf++);
         for (Uint i=0; i<elems.size(); ++i)
           elems[i] = *glb_elm_idx_buf++;
         CFinfo << "adding glb elem indexes at idx " << node_to_glb_elms_buffer[comp_idx]->add_row(elems) << CFendl;
         
         CFinfo << "adding glb node index at idx " << glb_node_indices_buffer[comp_idx]->add_row(globalIDs[IDX + id*gidSize]) << CFendl;
         
         
			 }
		 }
		
	}
  
	
	
  //static Zoltan_DD m_dd;
	//static std::vector<int> m_dd_parts;
  static std::set<Uint> m_ghost_nodes;
	
	int m_argc;
	char** m_argv;
};

//Zoltan_DD ZoltanTests_Fixture::m_dd;
//std::vector<int> ZoltanTests_Fixture::m_dd_parts;
std::set<Uint> ZoltanTests_Fixture::m_ghost_nodes;

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( ZoltanTests_TestSuite, ZoltanTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
	PE::instance().init(m_argc,m_argv);
}

//////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE ( zoltan_quadtriag_mesh)
{
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
	meshreader->configure_property("Read Boundaries",false);

	// the file to read from
	boost::filesystem::path fp_in ("quadtriag.neu");

	// the mesh to store in
	CMesh::Ptr mesh_ptr = meshreader->create_mesh_from(fp_in);
  CMesh& mesh = *mesh_ptr;

	// Zoltan
  ZoltanObject *zz = new ZoltanObject(PE::instance());
  if (zz == NULL)
    throw BadValue(FromHere(),"Zoltan error");

	
	std::string graph_package = "Parmetis";
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
  zz->Set_Param( "NUM_GLOBAL_PARTS", "4");
  zz->Set_Param( "CHECK_GRAPH", "2"); 
  //zz->Set_Param( "PHG_EDGE_SIZE_THRESHOLD", ".35");  // 0-remove all, 1-remove none

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
	
	
	Component::Ptr partition_info = mesh.create_component<Component>("temporary_partition_info");
	partition_info->properties()["node_start_idx"]=Uint(0);
	partition_info->properties()["elem_start_idx"]=mesh.property("nb_nodes").value<Uint>();
	Uint node_start_idx = mesh.get_child("temporary_partition_info")->property("node_start_idx").value<Uint>();
	Uint elem_start_idx = mesh.get_child("temporary_partition_info")->property("elem_start_idx").value<Uint>();


	BOOST_CHECK(true);
	
  
  
  
  int rc;  


 
  
  rc = zz->LB_Partition(changes, numGidEntries, numLidEntries,
    numImport, importGlobalIds, importLocalIds, importProcs, importToPart,
    numExport, exportGlobalIds, exportLocalIds, exportProcs, exportToPart);

	BOOST_CHECK(true);

  if (rc != (int)ZOLTAN_OK)
  {
    CFinfo << "Partitioning failed on process " << PE::instance().rank() << CFendl;
  }
	
	
	BOOST_CHECK(true);

	
	
	PE::instance().barrier();

	//PE_SERIALIZE
	//(
  CFinfo.setFilterRankZero(false);
	
	if (PE::instance().rank() == 0)
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
  
  
  give_elems_global_node_numbers(mesh);
	
  
  CFinfo << "before elems migration\n";
  CFinfo << "------------------" << CFendl;


	PE_SERIALIZE(
	BOOST_FOREACH(const CElements& elems, find_components_recursively<CElements>(mesh))
  {
    CFinfo << "# " << proc << "    nb_elems = " << elems.connectivity_table().size() << " in " << elems.full_path().string() <<  CFendl;
  }
  )

	zz->Set_Obj_Size_Multi_Fn ( get_elems_sizes , &mesh );
  zz->Set_Pack_Obj_Multi_Fn ( pack_elems_messages , &mesh );
  zz->Set_Unpack_Obj_Multi_Fn ( unpack_elems_messages , &mesh );
  zz->Set_Post_Migrate_PP_Fn ( post_migrate_elems , &mesh );


	BOOST_CHECK(true);


	rc = zz->Migrate( numImport, importGlobalIds, importLocalIds, importProcs, importToPart,
										numExport, exportGlobalIds, exportLocalIds, exportProcs, exportToPart);


	BOOST_CHECK(true);

  CFinfo << "after elems migration\n";
  CFinfo << "------------------" << CFendl;

	PE_SERIALIZE(
	BOOST_FOREACH(const CElements& elems, find_components_recursively<CElements>(mesh))
  {
    CFinfo << "# " << proc << " now contains " << elems.connectivity_table().size() << " elems in " << elems.full_path().string() <<  CFendl;
  }
  )
  
  
  CFinfo << "searching for ghost nodes\n";
  CFinfo << "-------------------------" <<CFendl;
  
  
  int num_known = m_ghost_nodes.size();
  ZOLTAN_ID_PTR known_global_ids = new Uint [num_known];
  ZOLTAN_ID_PTR known_local_ids = new Uint [num_known*2];
  int* known_procs = new int [num_known];
  int* known_to_part = new int [num_known];;

	int num_found;
	ZOLTAN_ID_PTR found_global_ids;// = importGlobalIds;
  ZOLTAN_ID_PTR found_local_ids;// = NULL;//  = importLocalIds;
	int* found_procs;// = importProcs;
  int* found_to_part;// = importToPart;


  Uint idx=0;
  BOOST_FOREACH(const Uint ghost_node, m_ghost_nodes)
  {
    known_global_ids[idx] = ghost_node;
    known_to_part[idx] = PE::instance().rank();
    //known_procs[idx] = ghost_node < 8 ? 0 : 1;
    ++idx;
  }

  PE_SERIALIZE(
  for (int i=0; i<num_known; ++i)
  {
    CFinfo << "#" << proc << " needs ghost node with global id " << known_global_ids[i] << CFendl;
  }
  )
  
  
  CFinfo << "before nodes migration\n";
  CFinfo << "------------------" << CFendl;
	
	zz->Set_Obj_Size_Multi_Fn ( get_nodes_sizes , &mesh );
  zz->Set_Pack_Obj_Multi_Fn ( pack_nodes_messages , &mesh );
  zz->Set_Unpack_Obj_Multi_Fn ( unpack_nodes_messages , &mesh );
  zz->Set_Post_Migrate_PP_Fn ( NULL , &mesh );
  
	
	BOOST_CHECK(true);
	
	
	rc = zz->Migrate( numImport, importGlobalIds, importLocalIds, importProcs, importToPart,
										numExport, exportGlobalIds, exportLocalIds, exportProcs, exportToPart);
	
	BOOST_CHECK(true);
	
  CFinfo << "after nodes migration\n";
  CFinfo << "------------------" << CFendl;

  rm_ghost_nodes(mesh);

  Zoltan_DD dd;

  int ierr; 
  Uint count = get_number_of_objects_mesh(&mesh, &ierr);



  ZOLTAN_ID_PTR globalID = new Uint [count];
  ZOLTAN_ID_PTR localID = new Uint [2*count];
  get_object_list_mesh(&mesh, 
                        1, 2,
                        globalID, localID,
                        0, 
                        NULL, 
                        &ierr);
  rc = dd.Create( PE::instance(),     // mpi comm
                   1,                  // length of global ID
                   2,                  // length of local ID
                   0,                  // length of user data
                   count,              // hash table size
                   0 );                // debug level      
  if (rc != (int)ZOLTAN_OK)
    throw InvalidStructure(FromHere(), "Could not create zoltan distributed directory");


  std::vector<int> dd_parts;
  dd_parts.resize(count,PE::instance().rank());
  rc = dd.Update(globalID,  // global IDs (ZOLTAN_ID_PTR)
                 localID,  // local IDs (ZOLTAN_ID_PTR)
                 NULL,  // user data  (ZOLTAN_ID_PTR)
                 &dd_parts[0],  // partition (int*)
                 count);    // count of objects
  if (rc != (int)ZOLTAN_OK)
    throw InvalidStructure(FromHere(), "Could not update zoltan distributed directory");

// WARNING: proc numbers are no longer correct!!!

  rc = dd.Find(known_global_ids, known_local_ids, NULL, 
               NULL, num_known, known_procs);

 	rc = zz->Invert_Lists ( num_known, known_global_ids, known_local_ids, known_procs, known_to_part, 
 													num_found, found_global_ids, found_local_ids, found_procs, found_to_part); 


  PE_SERIALIZE(
    CFinfo << "++++++++ ["<<proc<<"]  found " << num_found << " nodes to export as ghost" << CFendl;
    )



  // PE_SERIALIZE(
  // for (int i=0; i<num_found; ++i)
  // {
  //   CFinfo << "["<<proc<<"]  global id " << found_global_ids[i] << " must be sent to proc " << found_procs[i] << " to part " << found_to_part[i] << CFendl;
  //   CFinfo << "      it is located in " << found_local_ids[2*i+COMP] << " ("<<found_local_ids[2*i+IDX]<<")"<<CFendl;
  // }
  // )


    CFinfo << "before ghost nodes migration\n";
    CFinfo << "-----------------------------" << CFendl;

  	zz->Set_Obj_Size_Multi_Fn ( get_ghost_nodes_sizes , &mesh );
    zz->Set_Pack_Obj_Multi_Fn ( pack_ghost_nodes_messages , &mesh );
    zz->Set_Unpack_Obj_Multi_Fn ( unpack_ghost_nodes_messages , &mesh );
    zz->Set_Post_Migrate_PP_Fn ( NULL , &mesh );


  	BOOST_CHECK(true);


  	rc = zz->Migrate( num_known, known_global_ids, known_local_ids, known_procs, known_to_part, 
											num_found, found_global_ids, found_local_ids, found_procs, found_to_part);

  	BOOST_CHECK(true);

    CFinfo << "after ghost nodes migration\n";
    CFinfo << "------------------" << CFendl;


	mesh.remove_component("temporary_partition_info");
	partition_info.reset();


  ZoltanObject::LB_Free_Part(&known_global_ids, &known_local_ids, &known_procs, &known_to_part);
  ZoltanObject::LB_Free_Part(&found_global_ids, &found_local_ids, &found_procs, &found_to_part);  

  ZoltanObject::LB_Free_Part(&importGlobalIds, &importLocalIds, &importProcs, &importToPart);
  ZoltanObject::LB_Free_Part(&exportGlobalIds, &exportLocalIds, &exportProcs, &exportToPart);  

  delete globalID;
  delete localID;  

  delete zz;
	BOOST_CHECK(true);


  give_elems_local_node_numbers(mesh);
  
 	CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");

	// the file to read from
	boost::filesystem::path fp_out ("quadtriag.msh");

	meshwriter->write_from_to(mesh_ptr,fp_out);

} 
*/
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( CMeshPartitioner_test )
{
	CFinfo << "CMeshPartitioner_test" << CFendl;
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
	meshreader->configure_property("Read Boundaries",false);

	// the file to read from
	boost::filesystem::path fp_in ("quadtriag.neu");
	
	// the mesh to store in
	CMesh::Ptr mesh_ptr = meshreader->create_mesh_from(fp_in);
  CMesh& mesh = *mesh_ptr;

	
	CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
	boost::filesystem::path fp_out_1 ("quadtriag.msh");
	meshwriter->write_from_to(mesh_ptr,fp_out_1);
	
	
  CMeshPartitioner::Ptr partitioner_ptr = create_component_abstract_type<CMeshPartitioner>("CF.Mesh.Zoltan.CPartitioner","partitioner");

  CMeshPartitioner& p = *partitioner_ptr;
  
  //p.configure_property("Number of Partitions", (Uint) 4);
  p.configure_property("Graph Package", std::string("PHG"));
	p.configure_property("Debug Level", 10u);
  p.initialize(mesh);
  p.partition_graph();
  p.show_changes();
	
	p.migrate();
  
	boost::filesystem::path fp_out_2 ("quadtriag_repartitioned.msh");
	meshwriter->write_from_to(mesh_ptr,fp_out_2);
 	
	
	CFinfo << p.tree() << CFendl;
	
}

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
	PE::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

