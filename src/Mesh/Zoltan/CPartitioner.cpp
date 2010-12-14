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
#include "Common/MPI/tools.hpp"
#include "Common/Log.hpp"

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
  
  m_properties.add_option<OptionT <std::string> >("Graph Package","External library zoltan will use for graph partitioning","Parmetis");
  m_properties.add_option<OptionT <std::string> >("Debug Level","Internal Zoltan debug level (0 to 6)","6");
  
  m_zz = new ZoltanObject(PE::instance());
  cf_assert (m_zz != NULL);
}

//////////////////////////////////////////////////////////////////////////////

CPartitioner::~CPartitioner ( )
{  
	
	ZoltanObject::LB_Free_Part(&importGlobalIds, &importLocalIds, &importProcs, &importToPart);
  ZoltanObject::LB_Free_Part(&exportGlobalIds, &exportLocalIds, &exportProcs, &exportToPart);  

  delete m_zz;
}

//////////////////////////////////////////////////////////////////////////////

void CPartitioner::partition_graph()
{  
  set_partitioning_params();



  m_zz->LB_Partition(changes, numGidEntries, numLidEntries,
    numImport, importGlobalIds, importLocalIds, importProcs, importToPart,
    numExport, exportGlobalIds, exportLocalIds, exportProcs, exportToPart);
	
  m_changes->reserve(numExport);
  for (Uint i=0; i<(Uint)numExport; ++i)
  {
    m_changes->insert_blindly(exportGlobalIds[i],exportToPart[i]);
  }
  
  m_changes->sort_keys();
  
}

//////////////////////////////////////////////////////////////////////////////

void CPartitioner::set_partitioning_params()
{
	/// Zoltan parameters
  m_zz->Set_Param( "DEBUG_LEVEL", property("Debug Level").value<std::string>());
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
	
//////////////////////////////////////////////////////////////////////

void CPartitioner::migrate()
{
	//give_elems_global_node_numbers(mesh);
	
  
  CFinfo << "before elems migration\n";
  CFinfo << "----------------------" << CFendl;
	
	m_zz->Set_Obj_Size_Multi_Fn ( get_elems_sizes , this );
  m_zz->Set_Pack_Obj_Multi_Fn ( pack_elems_messages , this );
  m_zz->Set_Unpack_Obj_Multi_Fn ( unpack_elems_messages , this );
  m_zz->Set_Post_Migrate_PP_Fn ( post_migrate_elems , this );
	
	
	m_zz->Migrate( numImport, importGlobalIds, importLocalIds, importProcs, importToPart,
								 numExport, exportGlobalIds, exportLocalIds, exportProcs, exportToPart);
	
	
  CFinfo << "after elems migration\n";
  CFinfo << "---------------------" << CFendl;
	
  
  
  CFinfo << "searching for ghost nodes\n";
  CFinfo << "-------------------------" <<CFendl;
  
  
  int num_known = m_ghost_nodes.size();
  ZOLTAN_ID_PTR known_global_ids = new Uint [num_known];
  ZOLTAN_ID_PTR known_local_ids = NULL;
  int* known_procs = new int [num_known];
  int* known_to_part = new int [num_known];;
	
	int num_found;
	ZOLTAN_ID_PTR found_global_ids;// = importGlobalIds;
  ZOLTAN_ID_PTR found_local_ids = NULL;//  = importLocalIds;
	int* found_procs;// = importProcs;
  int* found_to_part;// = importToPart;
	
	
  Uint idx=0;
  BOOST_FOREACH(const Uint ghost_node, m_ghost_nodes)
  {
    known_global_ids[idx] = ghost_node;
    known_to_part[idx] = PE::instance().rank();
    known_procs[idx] = proc_of_obj(from_node_glb(ghost_node));
    ++idx;
  }
	
	
	m_zz->Invert_Lists ( num_known, known_global_ids, known_local_ids, known_procs, known_to_part, 
											 num_found, found_global_ids, found_local_ids, found_procs, found_to_part); 

	PEProcessSortedExecute(PE::instance(),-1,
												 CFinfo << CFendl << "proc#" << PE::instance().rank() << CFendl;
	CFinfo << "request ghost nodes:" << CFendl;
	for (Uint i=0; i<num_known; ++i)
	{
		CFinfo << known_global_ids[i] << " to part " << known_to_part[i] << " on proc " << known_procs[i] << CFendl;
	}
	
	CFinfo << "must send ghost nodes:" << CFendl;
	for (Uint i=0; i<num_found; ++i)
	{
		CFinfo << found_global_ids[i] << " to part " << found_to_part[i] << " on proc " << found_procs[i] << CFendl;
	}
												 CFinfo << CFendl;
												 )
	
  
	/*
  CFinfo << "before nodes migration\n";
  CFinfo << "------------------" << CFendl;
	
	m_zz->Set_Obj_Size_Multi_Fn ( get_nodes_sizes , &mesh );
  m_zz->Set_Pack_Obj_Multi_Fn ( pack_nodes_messages , &mesh );
  m_zz->Set_Unpack_Obj_Multi_Fn ( unpack_nodes_messages , &mesh );
  m_zz->Set_Post_Migrate_PP_Fn ( NULL , &mesh );
  
	
	m_zz->Migrate( numImport, importGlobalIds, importLocalIds, importProcs, importToPart,
								 numExport, exportGlobalIds, exportLocalIds, exportProcs, exportToPart);
	
	
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
	*/
	ZoltanObject::LB_Free_Part(&known_global_ids, &known_local_ids, &known_procs, &known_to_part);
  ZoltanObject::LB_Free_Part(&found_global_ids, &found_local_ids, &found_procs, &found_to_part);  

}
	
	
	//////////////////////////////////////////////////////////////////////

	void CPartitioner::get_elems_sizes(void *data, int gidSize, int lidSize, int num_ids,
											 ZOLTAN_ID_PTR globalIDs, ZOLTAN_ID_PTR localIDs, int *sizes, int *ierr)
	{
	  CFinfo << "++++++++++++++++++++++++++++++++++ get_elems_sizes"<<CFendl;
    
		CMeshPartitioner& p = *(CMeshPartitioner *)data;
		*ierr = ZOLTAN_OK;
		
		Component::Ptr comp;
		Uint idx;

		Uint nb_elems = 0;
		Uint i=0;
		foreach_container( (const Uint glb_idx) (const Uint part), p.changes() )
		{
			cf_assert(i<num_ids);
			if ( p.is_node(glb_idx) )
			{
				sizes[i] = 0;
			}
			else
			{
				nb_elems++;
				boost::tie(comp,idx) = p.to_local(glb_idx);
				sizes[i] = sizeof(Uint) // component index
								 + sizeof(Uint) * comp->as_type<CElements>()->connectivity_table().row_size(); // nodes

			}
			++i;
		}
		if (i != num_ids)
		{
			throw BadValue(FromHere(),"i!=num_ids");
		}
		CFLogVar(nb_elems);
		CFLogVar(num_ids);
	}
	
	void CPartitioner::pack_elems_messages(void *data, int gidSize, int lidSize, int num_ids,
													 ZOLTAN_ID_PTR globalIDs, ZOLTAN_ID_PTR localIDs, int *dests, int *sizes, int *idx, char *buf, int *ierr)
	{
		
    CFinfo << "++++++++++++++++++++++++++++++++++ begin pack_elems_messages"<<CFendl;
		CMeshPartitioner& p = *(CMeshPartitioner *)data;
		*ierr = ZOLTAN_OK;
		
		Uint comp_idx;
		Uint array_idx;
		bool is_found;
		
		CFinfo << "++++++++++++++++++++++++++++++++++++++++++++ give_elems_global_node_numbers" << CFendl;

		std::vector<boost::shared_ptr<CTable<Uint>::Buffer> > elem_buffer;
		boost_foreach (Component::Ptr comp, p.components_vector())
		{
			
			// give the element to node connectivity global indices
			if(CElements::Ptr elements = boost::dynamic_pointer_cast<CElements>(comp))
			{
				CTable<Uint>& conn_table = elements->connectivity_table();
				const CTable<Real>& coordinates = elements->coordinates();
				const CList<Uint>& global_node_indices = find_component_with_tag< CList<Uint> >(coordinates,"global_node_indices");
				
				BOOST_FOREACH ( CTable<Uint>::Row nodes, conn_table.array() )
				{
					BOOST_FOREACH ( Uint& node, nodes )
					{
						node = global_node_indices[node];
					}
				}
				elem_buffer.push_back(boost::shared_ptr<CTable<Uint>::Buffer> ( new CTable<Uint>::Buffer(elements->connectivity_table().create_buffer())));
			}
			else
				elem_buffer.push_back(boost::shared_ptr<CTable<Uint>::Buffer>());
		}
		
		Uint* comp_idx_buf;
		Uint* nodes_buf;
		
		Uint i=0;
		foreach_container( (const Uint glb_idx) (const Uint part), p.changes() )
		{
			if ( p.is_node(glb_idx) )
			{
				// don't pack anything
			}
			else
			{
				boost::tie(comp_idx,array_idx,is_found) = p.to_local_indices_from_glb_obj(glb_idx);
				
				comp_idx_buf = (Uint *)(buf + idx[i]);
				*comp_idx_buf++ = comp_idx;
				
				nodes_buf = (Uint *)(comp_idx_buf);
				boost_foreach (const Uint node, p.components_vector()[comp_idx]->as_type<CElements>()->connectivity_table()[array_idx])
			  {
          CFinfo << " " << node;
			    *nodes_buf++ = node;
			  }
        CFinfo << CFendl;
				
				elem_buffer[comp_idx]->rm_row(array_idx);
				
			}
			++i;
		}

		CFinfo << "++++++++++++++++++++++++++++++++++ end pack_elems_messages"<<CFendl;
    
	}
	
	
	
	void CPartitioner::unpack_elems_messages(void *data, int gidSize, int num_ids,
																		ZOLTAN_ID_PTR globalIDs, int *sizes, int *idx, char *buf, int *ierr)
	{
		
    CFinfo << "++++++++++++++++++++++++++++++++++ unpack_elems_messages"<<CFendl;
		CPartitioner& p = *(CPartitioner *)data;
		*ierr = ZOLTAN_OK;
		
		
		Uint comp_idx;
		
		std::vector<boost::shared_ptr<CTable<Uint>::Buffer> > elem_buffer;
		boost_foreach (Component::Ptr comp, p.components_vector())
		{
			if(CElements::Ptr elements = boost::dynamic_pointer_cast<CElements>(comp))
				elem_buffer.push_back(boost::shared_ptr<CTable<Uint>::Buffer> ( new CTable<Uint>::Buffer(elements->connectivity_table().create_buffer())));
			else
				elem_buffer.push_back(boost::shared_ptr<CTable<Uint>::Buffer>());
		}
		
		Uint* comp_idx_buf;
		Uint* nodes_buf;
		
		
		for (int id=0; id < num_ids; id++) 
		{
			int glb_obj = *(int*)(globalIDs+id*gidSize);
			
			if ( !p.is_node(glb_obj))
			{	
			  CFinfo << "+++++++ unpacking elem " << glb_obj << " : ";
				comp_idx_buf = (Uint *)(buf + idx[id]);
        comp_idx = *comp_idx_buf;
				
        Uint nb_nodes = p.components_vector()[comp_idx]->as_type<CElements>()->connectivity_table().row_size();
        std::vector<Uint> nodes(nb_nodes);
				
        nodes_buf = (Uint *)(++comp_idx_buf);
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
	
	void CPartitioner::post_migrate_elems(void *data, int gidSize, int lidSize,
																 int numImport, ZOLTAN_ID_PTR importGlobalID, ZOLTAN_ID_PTR importLocalID, int *importProc, int *importPart,
																 int numExport, ZOLTAN_ID_PTR exportGlobalID, ZOLTAN_ID_PTR exportLocalID, int *exportProc, int *exportPart,
																 int *ierr)
	{
		CPartitioner& p = *(CPartitioner *)data;
		*ierr = ZOLTAN_OK;
    CFinfo << "++++++++++++++++++++++++++++++++++ post_migrate_elems"<<CFendl;
		
    // may not rm_ghost-nodes here because then at node_migration, indexes are not valid
    //rm_ghost_nodes(mesh);
		
    std::set<Uint>::iterator it;
    std::set<Uint>::iterator not_found = m_ghost_nodes.end();
		
		
    // 1) put in ghost_nodes initially ALL the nodes required by the migrated elements
		boost_foreach (Component::Ptr comp, p.components_vector())
		{
			// give the element to node connectivity global indices
			if(CElements::Ptr elements = boost::dynamic_pointer_cast<CElements>(comp))
			{
				const CTable<Real>& coordinates = elements->coordinates();
				const CList<Uint>& global_node_indices = find_component_with_tag< CList<Uint> >(coordinates,"global_node_indices");
				CTable<Uint>& conn_table = elements->connectivity_table();
      
				boost_foreach ( CTable<Uint>::Row nodes, conn_table.array() )
				{
					boost_foreach ( Uint& node, nodes )
					{
						m_ghost_nodes.insert(node);
					}
				}
			}
		}
    
    CFinfo << "nodes after step 1 = ";
    boost_foreach (Uint node, m_ghost_nodes)
		CFinfo << " " << node ;
    CFinfo << CFendl;
    
    // 2) remove from ghost_nodes ALL the nodes that are present in the coordinate tables
		boost_foreach (Component::Ptr comp, p.components_vector())
		{
			// give the element to node connectivity global indices
			if(CElements::Ptr elements = boost::dynamic_pointer_cast<CElements>(comp))
			{
				const CTable<Real>& coordinates = elements->coordinates();
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
		}

    CFinfo << "nodes after step 2 = ";
    BOOST_FOREACH(Uint node, m_ghost_nodes)
		CFinfo << " " << node ;
    CFinfo << CFendl;
    
    // 3) add to ghost_nodes all the nodes that are going to be exported
    std::set<Uint> nodes_to_export;
    for (int id=0; id < numExport; id++) 
		{
			int glb_obj = *(int*)(exportGlobalID+id*gidSize);
			
			if ( p.is_node(glb_obj) ) // if it is a node
			{	
        it = m_ghost_nodes.find(glb_obj);
				
        if (it == not_found) // if not found
          m_ghost_nodes.insert(glb_obj);
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
			int glb_obj = *(int*)(importGlobalID+id*gidSize);
			
			if ( p.is_node(glb_obj) ) // if it is a node
			{	
        it = m_ghost_nodes.find(glb_obj);        
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
		
		CFinfo << "++++++++++++++++++++++++++++++++++++++++++++ give_elems_local_node_numbers" << CFendl;
    std::map<Uint,Uint> glb_to_loc;
    
		boost_foreach (Component::Ptr comp, p.components_vector())
		{
			// give the element to node connectivity global indices
			if(CElements::Ptr elements = boost::dynamic_pointer_cast<CElements>(comp))
			{
				const CTable<Real>& coordinates = elements->coordinates();
				const CList<Uint>& global_node_indices = find_component_with_tag< CList<Uint> >(coordinates,"global_node_indices");
				
				
				for (Uint i=0; i<coordinates.size(); ++i)
				{
					glb_to_loc[global_node_indices[i]]=i;
				}
			}
    }

		boost_foreach (Component::Ptr comp, p.components_vector())
		{
			
			// give the element to node connectivity global indices
			if(CElements::Ptr elements = boost::dynamic_pointer_cast<CElements>(comp))
			{
				CTable<Uint>& conn_table = elements->connectivity_table();
				
				BOOST_FOREACH ( CTable<Uint>::Row nodes, conn_table.array() )
				{
					BOOST_FOREACH ( Uint& node, nodes )
					{
						node = glb_to_loc[node];
					}
				}
			}    
			
		}
	}
	
	std::set<Uint> CPartitioner::m_ghost_nodes;

//////////////////////////////////////////////////////////////////////////////

} // Zoltan
} // Mesh
} // CF
