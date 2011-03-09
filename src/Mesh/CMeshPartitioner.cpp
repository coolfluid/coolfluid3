// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Foreach.hpp"
#include "Common/OptionT.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/MPI/tools.hpp"
#include "Common/Log.hpp"
#include "Common/StringConversion.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CMeshPartitioner.hpp"
#include "Mesh/CDynTable.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CRegion.hpp"

#include "Common/XML/Protocol.hpp"

namespace CF {
namespace Mesh {

  using namespace Common;
  using namespace Common::XML;

//////////////////////////////////////////////////////////////////////////////

CMeshPartitioner::CMeshPartitioner ( const std::string& name ) :
    CMeshTransformer(name),
    m_base(0),
    m_nb_parts(mpi::PE::instance().size()),
    m_map_built(false)
{
  m_properties.add_option<OptionT <Uint> >("nb_partitions","Number of Partitions","Total number of partitions (e.g. number of processors)",m_nb_parts)
    ->link_to(&m_nb_parts)
    ->mark_basic();
  
  m_global_to_local = allocate_component<CMap<Uint,Uint> >("global_to_local");
  add_static_component(m_global_to_local);

  m_changes = allocate_component<CMap<Uint,Uint> >("changes");
  add_static_component(m_changes);

  this->regist_signal ( "load_balance" , "partitions and migrates elements between processors", "Load Balance" )->connect ( boost::bind ( &CMeshPartitioner::load_balance,this, _1 ) );

  signal("load_balance").signature->connect( boost::bind(&CMeshPartitioner::load_balance_signature, this, _1));
}

//////////////////////////////////////////////////////////////////////////////

void CMeshPartitioner::execute()
{
  CMesh& mesh = *m_mesh.lock();
  initialize(mesh);
  partition_graph();
  show_changes();
  migrate();
}

//////////////////////////////////////////////////////////////////////////////

void CMeshPartitioner::load_balance( Signal::arg_t& node  )
{
	SignalFrame & options = node.map( Protocol::Tags::key_options() );

	URI path = options.get_option<URI>("Mesh");

	if( path.scheme() != URI::Scheme::CPATH )
		throw ProtocolError( FromHere(), "Wrong protocol to access the Domain component, expecting a \'cpath\' but got \'" + path.string() +"\'");

	// get the domain
  CMesh::Ptr mesh = access_component_ptr( path.path() )->as_ptr<CMesh>();
	if ( is_null(mesh) )
		throw CastingFailed( FromHere(), "Component in path \'" + path.string() + "\' is not a valid CMesh." );

	initialize(*mesh);

	partition_graph();

	migrate();
}

//////////////////////////////////////////////////////////////////////

void CMeshPartitioner::load_balance_signature ( Common::Signal::arg_t& node )
{
	SignalFrame & options = node.map( Protocol::Tags::key_options() );

	options.set_option<URI>("Mesh", URI(), "Mesh to load balance" );
}

//////////////////////////////////////////////////////////////////////

void CMeshPartitioner::initialize(CMesh& mesh)
{
  m_mesh = mesh.as_ptr<CMesh>();
  
  CList<bool>& node_is_ghost = mesh.nodes().is_ghost();
  Uint nb_ghost(0);
  for (Uint i=0; i<node_is_ghost.size(); ++i)
  {
    if (node_is_ghost[i])
      ++nb_ghost;
  }  
  
  Uint tot_nb_owned_nodes=mesh.nodes().size()-nb_ghost;
  Uint tot_nb_owned_elems=0;
  boost_foreach( CElements& elements, find_components_recursively<CElements>(mesh) )
  {
    tot_nb_owned_elems += elements.size();
  }
  
  std::vector<Uint> nb_nodes_per_proc(mpi::PE::instance().size());
  std::vector<Uint> nb_elems_per_proc(mpi::PE::instance().size());
//  boost::mpi::all_gather(world, tot_nb_owned_nodes, nb_nodes_per_proc);
//  boost::mpi::all_gather(world, tot_nb_owned_elems, nb_elems_per_proc);
  mpi::PE::instance().all_gather(&tot_nb_owned_nodes,1,(Uint*)(&nb_nodes_per_proc[0]));
  mpi::PE::instance().all_gather(&tot_nb_owned_elems,1,(Uint*)(&nb_elems_per_proc[0]));

  m_start_id_per_proc.resize(mpi::PE::instance().size());
  m_start_node_per_proc.resize(mpi::PE::instance().size());
  m_start_elem_per_proc.resize(mpi::PE::instance().size());
  m_end_id_per_proc.resize(mpi::PE::instance().size());
  m_end_node_per_proc.resize(mpi::PE::instance().size());
  m_end_elem_per_proc.resize(mpi::PE::instance().size());

  Uint start_id=0;
  for (Uint p=0; p<mpi::PE::instance().size(); ++p)
  {
    m_start_id_per_proc[p]   = start_id;
    m_end_id_per_proc[p]     = start_id + nb_nodes_per_proc[p] + nb_elems_per_proc[p];
    m_start_node_per_proc[p] = start_id;
    m_end_node_per_proc[p]   = start_id + nb_nodes_per_proc[p];
    m_start_elem_per_proc[p] = start_id + nb_nodes_per_proc[p];
    m_end_elem_per_proc[p]   = start_id + nb_nodes_per_proc[p] + nb_elems_per_proc[p];

    start_id += nb_nodes_per_proc[p]+nb_elems_per_proc[p];    
  }  

  build_global_to_local_index(mesh);
  build_graph();
}

//////////////////////////////////////////////////////////////////////////////

void CMeshPartitioner::build_global_to_local_index(CMesh& mesh)
{
  m_map_built = true;
  m_nb_owned_obj = 0;
  m_local_start_index.push_back(0);
  
  CNodes& nodes = mesh.nodes();
  CList<bool>& node_is_ghost = nodes.is_ghost();
  CList<Uint>& node_glb_idx = nodes.glb_idx();
  for (Uint i=0; i<nodes.size(); ++i)
  {
    if (!node_is_ghost[i])
    {
      //CFinfo << "owning node " << glb_idx << " --> " << from_node_glb(glb_idx) << CFendl;
      ++m_nb_owned_obj;
    }
  }
  m_local_components.push_back(mesh.nodes().self());
  m_local_start_index.push_back(mesh.nodes().size()+m_local_start_index.back());

  boost_foreach ( CEntities& elements, mesh.topology().elements_range() )
  {
    m_nb_owned_obj += elements.size();
    m_local_components.push_back(elements.self());
    m_local_start_index.push_back(elements.size()+m_local_start_index.back());
  }

  Uint tot_nb_obj = m_local_start_index.back();
  m_global_to_local->reserve(tot_nb_obj);
  Uint loc_idx=0;
  //CFinfo << "adding nodes to map " << CFendl;
  boost_foreach (Uint glb_idx, node_glb_idx.array())
  {
    m_global_to_local->insert_blindly(glb_idx,loc_idx++);
    //CFinfo << "  adding node with glb " << glb_idx << CFendl;
  }

  //CFinfo << "adding elements " << CFendl;
  boost_foreach ( CElements& elements, find_components_recursively<CElements>(mesh))
  {
    boost_foreach (Uint glb_idx, elements.glb_idx().array())
    {
      m_global_to_local->insert_blindly(glb_idx,loc_idx++);
      //CFinfo << "  adding element with glb " << glb_idx << CFendl;
    }
  }

  m_global_to_local->sort_keys();
}

//////////////////////////////////////////////////////////////////////////////

void CMeshPartitioner::show_changes()
{
  if (m_changes->size())
  {
    Component::Ptr component;
    Uint index;
    PEProcessSortedExecute(-1,
      std::cout << std::endl;
      std::cout << "proc #" << mpi::PE::instance().rank() << std::endl;
      std::cout << "-------" << std::endl;
      foreach_container((const Uint glb_obj) (const Uint part), *m_changes)
      {
        boost::tie(component,index) = to_local(glb_obj);

        if (is_node(glb_obj))
        {
          std::cout << "export node " << glb_obj << std::endl;
        }
        else
        {
          std::cout << "export elem " << glb_obj << std::endl;
        }
        //std::cout << "  to proc " << m_hash->proc_of_part(part) << std::endl;
        std::cout << "  to part " << part << std::endl;
        std::cout << "  from " << component->full_path().path() << "["<<index<<"]" << std::endl;
      }
    )
  }
  else
  {
    CFinfo << "No changes in partitions" << CFendl;
  }
}
//////////////////////////////////////////////////////////////////////////////

boost::tuple<Uint,Uint,bool> CMeshPartitioner::to_local_indices_from_glb_obj(const Uint glb_obj) const
{
  CMap<Uint,Uint>::const_iterator itr = m_global_to_local->find(glb_obj);
  if (itr != m_global_to_local->end() )
  {
    Uint loc_obj = itr->second;
    for (Uint i=0; i<m_local_start_index.size(); ++i)
    {
      if (loc_obj < m_local_start_index[i+1])
      {
        return boost::make_tuple(i,loc_obj-m_local_start_index[i],true);
      }
    }
  }
  return boost::make_tuple(0,0,false);
}

//////////////////////////////////////////////////////////////////////////////

boost::tuple<Uint,Uint> CMeshPartitioner::to_local_indices_from_loc_obj(const Uint loc_obj) const
{
  cf_assert_desc("loc_obj out of bounds" , loc_obj < m_local_start_index.back());
  for (Uint i=0; i<m_local_start_index.size()-1; ++i)
  {
    if (loc_obj < m_local_start_index[i+1])
    {
      return boost::make_tuple(i,loc_obj-m_local_start_index[i]);
    }
  }
  return boost::make_tuple(0,0);
}


//////////////////////////////////////////////////////////////////////////////

boost::tuple<Component::Ptr,Uint> CMeshPartitioner::to_local(const Uint glb_obj) const
{
  Uint component_idx;
  Uint loc_idx;
  bool found;
  boost::tie(component_idx,loc_idx,found) = to_local_indices_from_glb_obj(glb_obj);
  if (found)
    return boost::make_tuple(m_local_components[component_idx],loc_idx);
  else
  {
    throw ShouldNotBeHere(FromHere(), "No local obj for glb_obj [" + to_str(glb_obj) + "] found.");
    return boost::make_tuple(Component::Ptr(),0);
  }
}

//////////////////////////////////////////////////////////////////////////////

void CMeshPartitioner::migrate()
{
  // 1) wrap components to send
  // 2) pack
  // 3) remove from table
  // 4) mpi magic
  // 5) unpack
  // 6) request ghost nodes
  // 7) pack ghost nodes
  // 8) mpi magic
  // 9) unpack ghost nodes
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
