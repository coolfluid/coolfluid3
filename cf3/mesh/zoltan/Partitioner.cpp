// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Builder.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/StringConversion.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"
#include "common/Log.hpp"
#include "common/Map.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/zoltan/Partitioner.hpp"

namespace cf3 {
namespace mesh {
namespace zoltan {

  using namespace common;
  using namespace common::PE;

#define RANK "[" << Comm::instance().rank() << "] "

////////////////////////////////////////////////////////////////////////////////

cf3::common::ComponentBuilder < Partitioner, MeshTransformer, LibZoltan > zoltan_partitioner_transformer_builder;

//////////////////////////////////////////////////////////////////////////////

Partitioner::Partitioner ( const std::string& name ) :
  MeshPartitioner(name),
  m_partitioned(false)
{

  options().add_option("graph_package", "PHG")
      .description("External library zoltan will use for graph partitioning")
      .pretty_name("Graph Package")
      .mark_basic();

  options().add_option("debug_level", 0u)
      .description("Internal zoltan debug level (0 to 10)")
      .pretty_name("Debug Level");

  float version;
  int error_code = Zoltan_Initialize(Core::instance().argc(),Core::instance().argv(),&version);
  cf3_assert_desc("Could not initialize zoltan", error_code == ZOLTAN_OK);
  m_zoltan_version = version;

  properties().add_property("Zoltan_version",m_zoltan_version);
}

//////////////////////////////////////////////////////////////////////////////

Partitioner::~Partitioner ( )
{
  if (m_partitioned)
  {
    Zoltan::LB_Free_Part(&importGlobalIds, &importLocalIds, &importProcs, &importToPart);
    Zoltan::LB_Free_Part(&exportGlobalIds, &exportLocalIds, &exportProcs, &exportToPart);
  }

  // m_zz is deleted automatically by shared_ptr
}

Zoltan& Partitioner::zoltan_handle()
{
  cf3_assert( Comm::instance().is_initialized() );

  if ( is_null(m_zz) ) // create it
  {
    m_zz.reset( new Zoltan( Comm::instance().communicator() ) );
    cf3_assert (m_zz != nullptr);
  }

  return *m_zz;
}

//////////////////////////////////////////////////////////////////////////////

void Partitioner::partition_graph()
{
  CFdebug.setFilterRankZero(false);

  m_partitioned = true;
  set_partitioning_params();

  zoltan_handle().LB_Partition(changes, numGidEntries, numLidEntries,
    numImport, importGlobalIds, importLocalIds, importProcs, importToPart,
    numExport, exportGlobalIds, exportLocalIds, exportProcs, exportToPart);

  Uint comp; Uint loc_idx; bool found;
  for (Uint i=0; i<(Uint)numExport; ++i)
  {
    boost::tie(comp,loc_idx) = location_idx(exportGlobalIds[i]);
    if (comp == 0) // if is node
    {
      m_nodes_to_export[exportToPart[i]].push_back(loc_idx);
    }
    else // if is element
    {
      m_elements_to_export[comp-1][exportToPart[i]].push_back(loc_idx);
    }
  }

  // Import should not have been assigned
  // see line below: zoltan_handle().Set_Param( "RETURN_LISTS", "EXPORT");
  cf3_assert((int)numImport<=0);

  CFdebug.setFilterRankZero(true);

}

//////////////////////////////////////////////////////////////////////////////

void Partitioner::set_partitioning_params()
{
  /// zoltan general parameters

  zoltan_handle().Set_Param( "DEBUG_LEVEL", to_str( options()["debug_level"].value<Uint>() ));
  //  0 Quiet mode; no output unless an error or warning is produced.
  //  1 Values of all parameters set by Zoltan_Set_Param and used by zoltan.
  //  2 Timing information for zoltan's main routines.
  //  3 Timing information within zoltan's algorithms (support by algorithms is optional).
  //  4
  //  5 Trace information (enter/exit) for major zoltan interface routines (printed by the processor specified by the DEBUG_PROCESSOR parameter).
  //  6 Trace information (enter/exit) for major zoltan interface routines (printed by all processors).
  //  7 More detailed trace information in major zoltan interface routines.
  //  8 List of objects to be imported to and exported from each processor. [1]
  //  9
  // 10 Maximum debug output; may include algorithm-specific output. [1]
  //
  // [1] Output may be serialized; that is, one processor may have to complete its output before the next processor
  // is allowed to begin its output.  This serialization is not scalable and can significantly increase execution
  // time on large number of processors.


  zoltan_handle().Set_Param( "LB_METHOD", "GRAPH");
  // The load-balancing algorithm used by zoltan is specified by this parameter. Valid values are
  // BLOCK (for block partitioning),
  // RANDOM (for random partitioning),
  // RCB (for recursive coordinate bisection),
  // RIB (for recursive inertial bisection),
  // HSFC (for Hilbert space-filling curve partitioning),
  // REFTREE (for refinement tree based partitioning)
  // GRAPH (to choose from collection of methods for graphs),
  // HYPERGRAPH (to choose from a collection of methods for hypergraphs),
  // HIER (for hybrid hierarchical partitioning)
  // NONE (for no load balancing).

  zoltan_handle().Set_Param( "LB_APPROACH", "PARTITION");
  // The desired load balancing approach. Only LB_METHOD = HYPERGRAPH or GRAPH
  // uses the LB_APPROACH parameter. Valid values are
  //   PARTITION (Partition "from scratch," not taking into account the current data distribution;
  //             this option is recommended for static load balancing.)
  //   REPARTITION (Partition but take into account current data distribution to keep data migration low;
  //               this option is recommended for dynamic load balancing.)
  //   REFINE (Quickly improve the current data distribution.)

  zoltan_handle().Set_Param( "NUM_GID_ENTRIES", "1");
  // The number of unsigned integers that should be used to represent a global identifier (ID). Values greater than zero are accepted.

  zoltan_handle().Set_Param( "NUM_LID_ENTRIES", "0");
  // The number of unsigned integers that should be used to represent a local identifier (ID). Values greater than or equal to zero are accepted.

  zoltan_handle().Set_Param( "RETURN_LISTS", "EXPORT");
  // The lists returned by calls to Zoltan_LB_Partition. Valid values are
  // "IMPORT", to return only information about objects to be imported to a processor
  // "EXPORT", to return only information about objects to be exported from a processor
  // "ALL", or "IMPORT AND EXPORT" (or any string with both "IMPORT" and "EXPORT" in it) to return both import and export information
  // "PARTS" (or "PART ASSIGNMENT" or any string with "PART" in it) to return the new process and part assignment of every local object, including those not being exported.
  // "NONE", to return neither import nor export information


  zoltan_handle().Set_Param( "NUM_GLOBAL_PARTS", to_str( options()["nb_parts"].value<Uint>() ));
  // The total number of parts to be generated by a call to Zoltan_LB_Partition.


  /// zoltan graph parameters

  zoltan_handle().Set_Param( "GRAPH_PACKAGE", options()["graph_package"].value<std::string>() );
  // The software package to use in partitioning the graph.
  // PHG (default)     http://www.cs.sandia.gov/zoltan/ug_html/ug_alg_phg.html
  // ParMETIS          http://www.cs.sandia.gov/zoltan/ug_html/ug_alg_parmetis.html
  // Scotch/PT-Scotch  http://www.cs.sandia.gov/zoltan/ug_html/ug_alg_ptscotch.html

  if (m_zoltan_version >= 3.5)
  {
    zoltan_handle().Set_Param( "GRAPH_BUILD_TYPE","FAST_NO_DUP" );
    // Allow some optimizations in the graph building process:
    //   NORMAL      = graph is generic, no optimization can be performed
    //   FAST        = graph global IDs are in the interval [0,n-1], with IDs [0,a] on process 0, IDs [a+1, b] on process 1, IDs [b+1, c] on process 2, etc.
    //   FAST_NO_DUP = graph global IDs are in the interval [0,n-1], with IDs [0,a] on process 0, IDs [a+1, b] on process 1, IDs [b+1, c] on process 2, etc., and there are no duplicate edges and no need of symmetrization.
    //           See GRAPH_FAST_BUILD_BASE below to allow IDs to that are one-based instead of zero-based.
    // Default : NORMAL

    zoltan_handle().Set_Param( "GRAPH_FAST_BUILD_BASE","0" );
    // GRAPH_FAST_BUILD_BASE  When using GRAPH_BUILD_TYPE is FAST or FAST_NO_DUP,
    // IDs specified are in the range [GRAPH_FAST_BUILD_BASE, n-1+GRAPH_FAST_BUILD_BASE].
    // This parameter has no effect when GRAPH_BUILD_TYPE is NORMAL.
    // Default : GRAPH_FAST_BUILD_BASE = 0
  }


  zoltan_handle().Set_Param( "CHECK_GRAPH", to_str(std::max(2u, options()["debug_level"].value<Uint>() )));
  // Level of error checking for graph input:
  // 0 = no checking,
  // 1 = on-processor checking,
  // 2 = full checking. (CHECK_GRAPH==2 is very slow and should be used only during debugging).

  /// zoltan Query functions

  zoltan_handle().Set_Num_Obj_Fn(&Partitioner::query_nb_of_objects, this);
  zoltan_handle().Set_Obj_List_Fn(&Partitioner::query_list_of_objects, this);
  zoltan_handle().Set_Num_Edges_Multi_Fn(&Partitioner::query_nb_connected_objects, this);
  zoltan_handle().Set_Edge_List_Multi_Fn(&Partitioner::query_list_of_connected_objects, this);
}

//////////////////////////////////////////////////////////////////////////////

int Partitioner::query_nb_of_objects(void *data, int *ierr)
{
  MeshPartitioner& p = *(MeshPartitioner *)data;
  *ierr = ZOLTAN_OK;

  return p.nb_objects_owned_by_part(PE::Comm::instance().rank());
}

//////////////////////////////////////////////////////////////////////////////

void Partitioner::query_list_of_objects(void *data, int sizeGID, int sizeLID,
                                         ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                                         int wgt_dim, float *obj_wgts, int *ierr)
{
  MeshPartitioner& p = *(MeshPartitioner *)data;
  *ierr = ZOLTAN_OK;

  p.list_of_objects_owned_by_part(PE::Comm::instance().rank(),globalID);


  // for debugging
#if 0
  std::vector<Uint> glbID(p.nb_owned_objects());
  p.list_of_owned_objects(glbID);

  CFdebug << RANK << "glbID =";
  boost_foreach(const Uint g, glbID)
    CFdebug << " " << g;
  CFdebug << CFendl;
#endif

}

//////////////////////////////////////////////////////////////////////////////

void Partitioner::query_nb_connected_objects(void *data, int sizeGID, int sizeLID, int num_obj,
                                              ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                                              int *numEdges, int *ierr)
{
  MeshPartitioner& p = *(MeshPartitioner *)data;
  *ierr = ZOLTAN_OK;

  p.nb_connected_objects_in_part(PE::Comm::instance().rank(),numEdges);
}

//////////////////////////////////////////////////////////////////////////////

void Partitioner::query_list_of_connected_objects(void *data, int sizeGID, int sizeLID, int num_obj,
                                                   ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                                                   int *num_edges,
                                                   ZOLTAN_ID_PTR nborGID, int *nborProc,
                                                   int wgt_dim, float *ewgts, int *ierr)
{
  MeshPartitioner& p = *(MeshPartitioner *)data;
  *ierr = ZOLTAN_OK;

  p.list_of_connected_objects_in_part(PE::Comm::instance().rank(),nborGID);
  p.list_of_connected_procs_in_part(PE::Comm::instance().rank(),nborProc);



  // for debugging
#if 0
  const Uint nb_obj = p.nb_owned_objects();
  std::vector<Uint> numEdges(nb_obj);
  Uint tot_nb_edges = p.nb_connected_objects(numEdges);

  std::vector<Uint> conn_glbID(tot_nb_edges);
  p.list_of_connected_objects(conn_glbID);

  std::vector<Uint> glbID(nb_obj);
  p.list_of_owned_objects(glbID);

  PEProcessSortedExecute(-1,
  CFdebug << RANK << "conn_glbID =\n";
  Uint cnt = 0;
  index_foreach( e, const Uint nbEdge, numEdges)
  {
    if (p.is_node(glbID[e]))
      CFdebug << "  N" << p.to_node_glb(glbID[e]) << ": ";
    else
      CFdebug << "  E" << p.to_elem_glb(glbID[e]) << ": ";

    for (Uint c=cnt; c<cnt+nbEdge; ++c)
    {
      if (p.is_node(conn_glbID[c]))
        CFdebug << " N" << p.to_node_glb(conn_glbID[c]);
      else
        CFdebug << " E" << p.to_elem_glb(conn_glbID[c]);
    }
    CFdebug << "\n";
    cnt += nbEdge;
  }
  CFdebug << CFendl;
  cf3_assert( cnt == tot_nb_edges );
  CFdebug << RANK << "total nb_edges = " << cnt << CFendl;
  )
#endif

}

//////////////////////////////////////////////////////////////////////////////

#undef RANK

} // zoltan
} // mesh
} // cf3
