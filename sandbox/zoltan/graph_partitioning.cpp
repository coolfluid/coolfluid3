#include <boost/assign/list_of.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/mpi/collectives.hpp>

#include <mpi.h>
#include <zoltan_cpp.h>

#include "Common/CF.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/MPI/PE.hpp"

using namespace CF;
using namespace CF::Common;
using namespace boost::assign;
//using namespace CF::Mesh;

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

int* to_ptr(std::vector<int>& vec)
{
	if (vec.empty())
		return NULL;
	else
		return &vec[0];
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

void showGraphPartitions(GRAPH_DATA& graph, std::vector<int>& parts)
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


int get_number_of_objects(void *data, int *ierr)
{
  *ierr = ZOLTAN_OK;
  GRAPH_DATA& graph = *(GRAPH_DATA *)data;
  return graph.globalID.size();
}

void get_object_list(void *data, int sizeGID, int sizeLID,
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

void get_num_edges_list(void *data, int sizeGID, int sizeLID,
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

void get_edges_list(void *data, int sizeGID, int sizeLID,
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

int main(int argc, char** argv)
{
  PE::instance().init(argc,argv);
	if (PE::instance().size() != 3)
	{
    CFinfo << "must run this testcase with 3 processors" << CFendl;
	}
	else
	{
	  
		boost::shared_ptr<GRAPH_DATA> graph_ptr = build_graph();
		//boost::shared_ptr<GRAPH_DATA> graph_ptr = build_element_node_graph();
		GRAPH_DATA& graph = *graph_ptr;
		
		
		
		Zoltan *zz = new Zoltan(PE::instance());
		if (zz == NULL)
			throw BadValue(FromHere(),"Zoltan error");

		std::string graph_package = "PHG";
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

		// Graph parameters 

		zz->Set_Param( "NUM_GLOBAL_PARTS", "3");
		zz->Set_Param( "CHECK_GRAPH", "2"); 
		zz->Set_Param( "PHG_EDGE_SIZE_THRESHOLD", ".35");  // 0-remove all, 1-remove none 

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
  return 0;
}
