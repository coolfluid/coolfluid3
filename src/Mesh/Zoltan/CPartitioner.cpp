// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// coolfluid
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/String/Conversion.hpp"
#include "Common/MPI/PE.hpp"

#include "Mesh/Zoltan/CPartitioner.hpp"

namespace CF {
namespace Mesh {
namespace Zoltan {

  using namespace Common;
  using namespace Common::String;

////////////////////////////////////////////////////////////////////////////////

CF::Common::ComponentBuilder < CPartitioner, CMeshPartitioner, LibZoltan > zoltan_partitioner_builder;

//////////////////////////////////////////////////////////////////////////////

CPartitioner::CPartitioner ( const std::string& name ) : 
    CMeshPartitioner(name)
{
  
  m_properties.add_option<OptionT <std::string> >("Graph Package","External library zoltan will use for graph partitioning","SCOTCH");
  
  m_zz = new ZoltanObject(PE::instance());
  cf_assert (m_zz != NULL);
}

//////////////////////////////////////////////////////////////////////////////

CPartitioner::~CPartitioner ( )
{  
  delete m_zz;
}

//////////////////////////////////////////////////////////////////////////////

void CPartitioner::partition_graph()
{  
  set_partitioning_params();

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

  int rc = m_zz->LB_Partition(changes, numGidEntries, numLidEntries,
    numImport, importGlobalIds, importLocalIds, importProcs, importToPart,
    numExport, exportGlobalIds, exportLocalIds, exportProcs, exportToPart);

  m_changes->reserve(numExport);
  for (Uint i=0; i<numExport; ++i)
  {
    m_changes->insert_blindly(exportGlobalIds[i],exportToPart[i]);
  }
  
  m_changes->sort_keys();
  
  ZoltanObject::LB_Free_Part(&importGlobalIds, &importLocalIds, &importProcs, &importToPart);
  ZoltanObject::LB_Free_Part(&exportGlobalIds, &exportLocalIds, &exportProcs, &exportToPart);  
}

//////////////////////////////////////////////////////////////////////////////

void CPartitioner::set_partitioning_params()
{
	/// Zoltan parameters
  m_zz->Set_Param( "DEBUG_LEVEL", "0");
  m_zz->Set_Param( "LB_METHOD", "GRAPH");
  m_zz->Set_Param( "GRAPH_PACKAGE",property("Graph Package").value<std::string>());
  m_zz->Set_Param( "LB_APPROACH", "PARTITION");
  m_zz->Set_Param( "NUM_GID_ENTRIES", "1"); 
  m_zz->Set_Param( "NUM_LID_ENTRIES", "0");
  m_zz->Set_Param( "RETURN_LISTS", "EXPORT");
  m_zz->Set_Param( "GRAPH_SYMMETRIZE","NONE");

  // Graph parameters
  m_zz->Set_Param( "NUM_GLOBAL_PARTS", to_str(property("Number of Partitions").value<Uint>()));
  m_zz->Set_Param( "CHECK_GRAPH", "2"); 

	// Query functions 
  m_zz->Set_Num_Obj_Fn(&CPartitioner::query_nb_of_objects, this);
  m_zz->Set_Obj_List_Fn(&CPartitioner::query_list_of_objects, this);
  m_zz->Set_Num_Edges_Multi_Fn(&CPartitioner::query_nb_connected_objects, this);
  m_zz->Set_Edge_List_Multi_Fn(&CPartitioner::query_list_of_connected_objects, this);
}

//////////////////////////////////////////////////////////////////////////////

int CPartitioner::query_nb_of_objects(void *data, int *ierr)
{
  CMeshPartitioner& p = *(CMeshPartitioner *)data;
  *ierr = ZOLTAN_OK;
  
  return p.nb_owned_objects();
}

//////////////////////////////////////////////////////////////////////////////

void CPartitioner::query_list_of_objects(void *data, int sizeGID, int sizeLID,
                                         ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                                         int wgt_dim, float *obj_wgts, int *ierr)
{
  CMeshPartitioner& p = *(CMeshPartitioner *)data;
  *ierr = ZOLTAN_OK;
  
  p.list_of_owned_objects(globalID);
}

//////////////////////////////////////////////////////////////////////////////

void CPartitioner::query_nb_connected_objects(void *data, int sizeGID, int sizeLID, int num_obj,
                                              ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                                              int *numEdges, int *ierr)
{
  CMeshPartitioner& p = *(CMeshPartitioner *)data;
  *ierr = ZOLTAN_OK;
  
  p.nb_connected_objects(numEdges);
}

//////////////////////////////////////////////////////////////////////////////

void CPartitioner::query_list_of_connected_objects(void *data, int sizeGID, int sizeLID, int num_obj, 
                                                   ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                                                   int *num_edges,
                                                   ZOLTAN_ID_PTR nborGID, int *nborProc,
                                                   int wgt_dim, float *ewgts, int *ierr)
{
  CMeshPartitioner& p = *(CMeshPartitioner *)data;
  *ierr = ZOLTAN_OK;
  
  p.list_of_connected_objects(nborGID);
  p.list_of_connected_procs(nborProc);
}

//////////////////////////////////////////////////////////////////////////////

} // Zoltan
} // Mesh
} // CF
