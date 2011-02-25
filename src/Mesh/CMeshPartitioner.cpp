// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Foreach.hpp"
#include "Common/OptionT.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/MPI/operations.hpp"
#include "Common/MPI/all_reduce.hpp"
#include "Common/Log.hpp"
#include "Common/StringConversion.hpp"
#include "Common/MPI/tools.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CMeshPartitioner.hpp"
#include "Mesh/CDynTable.hpp"
#include "Mesh/CNodes.hpp"

#include "Common/XML/Protocol.hpp"

namespace CF {
namespace Mesh {

  using namespace Common;
  using namespace Common::XML;

//////////////////////////////////////////////////////////////////////////////

CMeshPartitioner::CMeshPartitioner ( const std::string& name ) :
    Component(name),
    m_base(0),
    m_nb_parts(mpi::PE::instance().size()),
    m_map_built(false)
{
  m_properties.add_option<OptionT <Uint> >("Number of Partitions","Total number of partitions (e.g. number of processors)",m_nb_parts);

  m_properties["Number of Partitions"].as_option().attach_trigger ( boost::bind ( &CMeshPartitioner::config_nb_parts,   this ) );

  m_hash = allocate_component<CMixedHash>("hash");
  add_static_component(m_hash);

  m_global_to_local = allocate_component<CMap<Uint,Uint> >("global_to_local");
  add_static_component(m_global_to_local);

  m_changes = allocate_component<CMap<Uint,Uint> >("changes");
  add_static_component(m_changes);

  this->regist_signal ( "load_balance" , "partitions and migrates elements between processors", "Load Balance" )->connect ( boost::bind ( &CMeshPartitioner::load_balance,this, _1 ) );

  signal("load_balance").signature->connect( boost::bind(&CMeshPartitioner::load_balance_signature, this, _1));
}

//////////////////////////////////////////////////////////////////////////////

void CMeshPartitioner::config_nb_parts()
{
  property("Number of Partitions").put_value(m_nb_parts);
//	m_hash->configure_property("Number of Partitions",m_nb_parts);
}

//////////////////////////////////////////////////////////////////////////////

void CMeshPartitioner::load_balance( Signal::arg_t& node  )
{
	SignalFrame & options = node.map( Protocol::Tags::key_options() );

	URI path = options.get_option<URI>("Mesh");

	if( path.scheme() != URI::Scheme::CPATH )
		throw ProtocolError( FromHere(), "Wrong protocol to access the Domain component, expecting a \'cpath\' but got \'" + path.string() +"\'");

	// get the domain
	CMesh::Ptr mesh = access_component_ptr<CMesh>( path.path() );
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

  std::vector<Uint> num_obj(2);
  num_obj[0] = mesh.property("nb_nodes").value<Uint>();
  num_obj[1] = mesh.property("nb_cells").value<Uint>();
  m_hash->configure_property("Number of Objects", num_obj);
  build_global_to_local_index(mesh);
  build_graph();
}

//////////////////////////////////////////////////////////////////////////////

void CMeshPartitioner::build_global_to_local_index(CMesh& mesh)
{
  m_map_built = true;
  m_nb_owned_obj = 0;
  m_local_start_index.push_back(0);
  boost_foreach ( CNodes& nodes, find_components_recursively<CNodes>(mesh) )
  {
//    const CList<bool>& is_ghost = find_component_with_tag<CList<bool> >(coordinates,"is_ghost");
//    boost_foreach (bool is_ghost_entry, is_ghost.array())
//      if (!is_ghost_entry)
//        ++m_nb_owned_obj;

    boost_foreach (Uint glb_idx, nodes.glb_idx().array())
    {
      if (m_hash->owns(from_node_glb(glb_idx)))
      {
        //CFinfo << "owning node " << glb_idx << " --> " << from_node_glb(glb_idx) << CFendl;
        ++m_nb_owned_obj;
      }
    }
    m_local_components.push_back(nodes.self());
    m_local_start_index.push_back(nodes.size()+m_local_start_index.back());
  }

  boost_foreach ( CElements& elements, find_components_recursively<CElements>(mesh))
  {
    m_nb_owned_obj += elements.size();

    // boost_foreach (Uint glb_idx, global_elem_indices.array())
    // {
    //   if (m_hash->subhash(ELEMS)->owns(glb_idx))
    //   {
    //     CFinfo << "owning elem " << glb_idx << " --> " << from_elem_glb(glb_idx) << CFendl;
    //   }
    //   else
    //   {
    //     CFinfo << "not owning elem " <<  glb_idx << " --> " << from_elem_glb(glb_idx) << CFendl;
    //   }
    // }
    m_local_components.push_back(elements.self());
    m_local_start_index.push_back(elements.size()+m_local_start_index.back());

  }

  Uint tot_nb_obj = m_local_start_index.back();
  m_global_to_local->reserve(tot_nb_obj);
  Uint loc_idx=0;
  //CFinfo << "adding nodes to map " << CFendl;
  boost_foreach ( CNodes& nodes, find_components_recursively<CNodes>(mesh) )
  {
    boost_foreach (Uint glb_idx, nodes.glb_idx().array())
    {
      m_global_to_local->insert_blindly(from_node_glb(glb_idx),loc_idx++);
      //CFinfo << "  adding node with glb " << from_node_glb(glb_idx) << CFendl;
    }
  }
  //CFinfo << "adding elements " << CFendl;
  boost_foreach ( CElements& elements, find_components_recursively<CElements>(mesh))
  {
    boost_foreach (Uint glb_idx, elements.glb_idx().array())
    {
      m_global_to_local->insert_blindly(from_elem_glb(glb_idx),loc_idx++);
      //CFinfo << "  adding element with glb " << from_elem_glb(glb_idx) << CFendl;
    }
  }

  m_global_to_local->sort_keys();

  // check validity
  cf_assert(loc_idx == tot_nb_obj);
  Uint glb_nb_owned_obj;
  mpi::all_reduce(mpi::PE::instance(), mpi::plus(), &m_nb_owned_obj, 1, &glb_nb_owned_obj);
  cf_assert(glb_nb_owned_obj == mesh.property("nb_nodes").value<Uint>() + mesh.property("nb_cells").value<Uint>());
}

//////////////////////////////////////////////////////////////////////////////

void CMeshPartitioner::show_changes()
{
  if (m_changes->size())
  {
    Component::Ptr component;
    Uint index;
    PEProcessSortedExecute(mpi::PE::instance(),-1,
      std::cout << std::endl;
      std::cout << "proc #" << mpi::PE::instance().rank() << std::endl;
      std::cout << "-------" << std::endl;
      foreach_container((const Uint glb_obj) (const Uint part), *m_changes)
      {
        boost::tie(component,index) = to_local(glb_obj);

        if (is_node(glb_obj))
        {
          std::cout << "export node " << to_node_glb(glb_obj) << std::endl;
        }
        else
        {
          std::cout << "export elem " << to_elem_glb(glb_obj) << std::endl;
        }
        std::cout << "  to proc " << m_hash->proc_of_part(part) << std::endl;
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
