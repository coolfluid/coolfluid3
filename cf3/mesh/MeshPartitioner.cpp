// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/datatype.hpp"
#include "common/PE/Buffer.hpp"
#include "common/PE/debug.hpp"
#include "common/StringConversion.hpp"
#include "common/DynTable.hpp"
#include "common/List.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/MeshPartitioner.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Region.hpp"
#include "mesh/MeshAdaptor.hpp"
#include "mesh/MeshElements.hpp"

namespace cf3 {
namespace mesh {


  using namespace common;
  using namespace common::XML;
  using namespace common::PE;

//////////////////////////////////////////////////////////////////////////////

MeshPartitioner::MeshPartitioner ( const std::string& name ) :
    MeshTransformer(name),
    m_base(0),
    m_nb_parts(PE::Comm::instance().size())
{
  options().add("nb_parts", m_nb_parts)
      .description("Total number of partitions (e.g. number of processors)")
      .pretty_name("Number of Partitions")
      .link_to(&m_nb_parts)
      .mark_basic();

  m_global_to_local = create_static_component<common::Map<Uint,Uint> >("global_to_local");
  m_lookup = create_static_component<UnifiedData >("lookup");

  regist_signal( "load_balance" )
    .description("Partitions and migrates elements between processors")
    .pretty_name("Load Balance")
    .connect ( boost::bind ( &MeshPartitioner::load_balance,this, _1 ) )
    .signature ( boost::bind ( &MeshPartitioner::load_balance_signature, this, _1));
}

//////////////////////////////////////////////////////////////////////////////

void MeshPartitioner::execute()
{
  Mesh& mesh = *m_mesh;
  initialize(mesh);
  Comm::instance().barrier();
  CFdebug << "    -partitioning" << CFendl;
  partition_graph();
//  show_changes();
  Comm::instance().barrier();
  CFdebug << "    -migrating" << CFendl;
  migrate();
}

//////////////////////////////////////////////////////////////////////////////

void MeshPartitioner::load_balance( SignalArgs& node  )
{
  SignalOptions options( node );

  URI path = options.value<URI>("mesh");

  if( path.scheme() != URI::Scheme::CPATH )
    throw ProtocolError( FromHere(), "Wrong protocol to access the Domain component, expecting a \'cpath\' but got \'" + path.string() +"\'");

  // get the domain
  Handle<Mesh> mesh(access_component(path));
  if ( is_null(mesh) )
    throw CastingFailed( FromHere(), "Component in path \'" + path.string() + "\' is not a valid Mesh." );

  m_mesh = mesh;

  execute();

}

//////////////////////////////////////////////////////////////////////

void MeshPartitioner::load_balance_signature ( common::SignalArgs& node )
{
  SignalOptions options( node );

  options.add("mesh", URI())
      .description("Mesh to load balance");
}

//////////////////////////////////////////////////////////////////////

void MeshPartitioner::initialize(Mesh& mesh)
{
  m_mesh = mesh.handle<Mesh>();
  Dictionary& nodes = mesh.geometry_fields();
  Uint tot_nb_owned_nodes(0);
  for (Uint i=0; i<nodes.size(); ++i)
  {
    if (nodes.is_ghost(i) == false)
      ++tot_nb_owned_nodes;
  }

  Uint tot_nb_owned_elems(0);
  boost_foreach( Elements& elements, find_components_recursively<Elements>(mesh) )
  {
    tot_nb_owned_elems += elements.size();
  }

  Uint tot_nb_owned_obj = tot_nb_owned_nodes + tot_nb_owned_elems;

  std::vector<Uint> nb_nodes_per_proc(PE::Comm::instance().size());
  std::vector<Uint> nb_elems_per_proc(PE::Comm::instance().size());
  std::vector<Uint> nb_obj_per_proc(PE::Comm::instance().size());
  PE::Comm::instance().all_gather(tot_nb_owned_nodes, nb_nodes_per_proc);
  PE::Comm::instance().all_gather(tot_nb_owned_elems, nb_elems_per_proc);
  PE::Comm::instance().all_gather(tot_nb_owned_obj, nb_obj_per_proc);
  m_start_id_per_part.resize(PE::Comm::instance().size());
  m_start_node_per_part.resize(PE::Comm::instance().size());
  m_start_elem_per_part.resize(PE::Comm::instance().size());
  m_end_id_per_part.resize(PE::Comm::instance().size());
  m_end_node_per_part.resize(PE::Comm::instance().size());
  m_end_elem_per_part.resize(PE::Comm::instance().size());

  Uint start_id(0);
  for (Uint p=0; p<PE::Comm::instance().size(); ++p)
  {
    m_start_id_per_part[p]   = start_id;
    m_end_id_per_part[p]     = start_id + nb_obj_per_proc[p];
    m_start_node_per_part[p] = start_id;
    m_end_node_per_part[p]   = start_id + nb_nodes_per_proc[p];
    m_start_elem_per_part[p] = start_id + nb_nodes_per_proc[p];
    m_end_elem_per_part[p]   = start_id + nb_nodes_per_proc[p] + nb_elems_per_proc[p];

    start_id += nb_obj_per_proc[p];
  }

  m_nodes_to_export.resize(m_nb_parts);
  m_elements_to_export.resize(m_nb_parts,std::vector< std::vector<Uint> >(mesh.elements().size()));

  build_global_to_local_index(mesh);
  build_graph();

//  mesh.update_statistics();
//  mesh.mesh_elements().update();
//  cf3_assert(m_mesh->elements().size() == m_mesh->mesh_elements().components().size());
//  for (Uint c=0; c<m_mesh->mesh_elements().components().size(); ++c)
//  {
//    cf3_assert(m_mesh->elements()[c]->handle() == m_mesh->mesh_elements().components()[c]);
//  }
}

//////////////////////////////////////////////////////////////////////////////

void MeshPartitioner::build_global_to_local_index(Mesh& mesh)
{
  Dictionary& nodes = mesh.geometry_fields();

  m_lookup->add(nodes);
  boost_foreach ( const Handle<Entities>& elements, mesh.elements() )
    m_lookup->add(*elements);

  m_nb_owned_obj = 0;
  common::List<Uint>& node_glb_idx = nodes.glb_idx();
  for (Uint i=0; i<nodes.size(); ++i)
  {
    if (!nodes.is_ghost(i))
    {
      //std::cout << PE::Comm::instance().rank() << " --   owning node " << node_glb_idx[i] << std::endl;
      ++m_nb_owned_obj;
    }
  }

  boost_foreach ( const Handle<Entities>& elements, mesh.elements() )
  {
    m_nb_owned_obj += elements->size();
  }

  Uint tot_nb_obj = m_lookup->size();
  m_global_to_local->reserve(tot_nb_obj);
  Uint loc_idx=0;
  //CFinfo << "adding nodes to map " << CFendl;
  boost_foreach (Uint glb_idx, node_glb_idx.array())
  {
    //CFinfo << "  adding node with glb " << glb_idx << CFendl;
    if (nodes.is_ghost(loc_idx) == false)
    {
      cf3_assert(glb_idx >= m_start_id_per_part[PE::Comm::instance().rank()]);
      cf3_assert(glb_idx >= m_start_node_per_part[PE::Comm::instance().rank()]);
      cf3_assert_desc(to_str(glb_idx)+">="+to_str(m_end_id_per_part[PE::Comm::instance().rank()]),glb_idx < m_end_id_per_part[PE::Comm::instance().rank()]);
      cf3_assert_desc(to_str(glb_idx)+">="+to_str(m_end_node_per_part[PE::Comm::instance().rank()]),glb_idx < m_end_node_per_part[PE::Comm::instance().rank()]);
    }
    else
    {
      cf3_assert(glb_idx > m_start_id_per_part[PE::Comm::instance().rank()] ||
                glb_idx <= m_end_id_per_part[PE::Comm::instance().rank()]);

      cf3_assert(glb_idx > m_start_node_per_part[PE::Comm::instance().rank()] ||
                glb_idx <= m_end_node_per_part[PE::Comm::instance().rank()]);
    }

    m_global_to_local->push_back(glb_idx,loc_idx++);
  }

  //CFinfo << "adding elements " << CFendl;
  boost_foreach ( const Handle<Entities>& elements, mesh.elements() )
  {
    boost_foreach (Uint glb_idx, elements->glb_idx().array())
    {
      cf3_assert_desc(to_str(glb_idx)+"<"+to_str(m_start_elem_per_part[PE::Comm::instance().rank()]),glb_idx >= m_start_elem_per_part[PE::Comm::instance().rank()]);
      cf3_assert_desc(to_str(glb_idx)+">="+to_str(m_end_elem_per_part[PE::Comm::instance().rank()]),glb_idx < m_end_elem_per_part[PE::Comm::instance().rank()]);
      cf3_assert_desc(to_str(glb_idx)+">="+to_str(m_end_id_per_part[PE::Comm::instance().rank()]),glb_idx < m_end_id_per_part[PE::Comm::instance().rank()]);
      m_global_to_local->push_back(glb_idx,loc_idx++);
      //CFinfo << "  adding element with glb " << glb_idx << CFendl;
    }
  }

  m_global_to_local->sort_keys();
}

//////////////////////////////////////////////////////////////////////////////

void MeshPartitioner::show_changes()
{
  Uint nb_changes(0);
  boost_foreach(std::vector<Uint>& export_nodes_to_part, m_nodes_to_export)
    nb_changes += export_nodes_to_part.size();

  boost_foreach(std::vector<std::vector<Uint> >& export_elems_to_part, m_elements_to_export)
    boost_foreach(std::vector<Uint>& export_elems_to_part_from_region, export_elems_to_part)
      nb_changes += export_elems_to_part_from_region.size();

  if (nb_changes > 0)
  {
    PEProcessSortedExecute(-1,
      for (Uint to_part=0; to_part<m_nodes_to_export.size(); ++to_part)
      {
        std::cout << "[" << PE::Comm::instance().rank() << "] export nodes to part " << to_part << ":  ";
        for (Uint n=0; n<m_nodes_to_export[to_part].size(); ++n)
          std::cout << m_nodes_to_export[to_part][n] << " ";
        std::cout << "\n";
      }
      for (Uint to_part=0; to_part<m_elements_to_export.size(); ++to_part)
      {
        for (Uint comp=0; comp<m_elements_to_export[to_part].size(); ++comp)
        {
          cf3_assert(comp+1 < m_lookup->components().size());
          std::string elements = m_lookup->components()[comp+1]->uri().path();
          std::cout << "[" << PE::Comm::instance().rank() << "] export " << elements << " to part " << to_part << ":  ";
          for (Uint e=0; e<m_elements_to_export[to_part][comp].size(); ++e)
            std::cout << m_elements_to_export[to_part][comp][e] << " ";
          std::cout << "\n";
        }
      }
    )
  }
  else
  {
    CFinfo << "No changes in partitions" << CFendl;
  }
}

//////////////////////////////////////////////////////////////////////////////

boost::tuple<Uint,Uint> MeshPartitioner::location_idx(const Uint glb_obj) const
{
  common::Map<Uint,Uint>::const_iterator itr = m_global_to_local->find(glb_obj);
  if (itr != m_global_to_local->end() )
  {
    return m_lookup->location_idx(itr->second);
  }
  return boost::make_tuple(0,0);
}

//////////////////////////////////////////////////////////////////////////////

boost::tuple<Handle< Component >,Uint> MeshPartitioner::location(const Uint glb_obj) const
{
  return m_lookup->location( (*m_global_to_local)[glb_obj] );
}

//////////////////////////////////////////////////////////////////////////////

void flex_all_to_all(const std::vector<PE::Buffer>& send, PE::Buffer& recv)
{
  std::vector<int> send_strides(send.size());
  std::vector<int> send_displs(send.size());
  for (Uint i=0; i<send.size(); ++i)
    send_strides[i] = send[i].size();

  if (send.size()) send_displs[0] = 0;
  for (Uint i=1; i<send.size(); ++i)
    send_displs[i] = send_displs[i-1] + send_strides[i-1];

  PE::Buffer send_linear;

  send_linear.reserve(send_displs.back()+send_strides.back());
  for (Uint i=0; i<send.size(); ++i)
    send_linear.pack(send[i].buffer(),send[i].size());

  std::vector<int> recv_strides(PE::Comm::instance().size());
  std::vector<int> recv_displs(PE::Comm::instance().size());
  PE::Comm::instance().all_to_all(send_strides,recv_strides);
  if (recv_displs.size()) recv_displs[0] = 0;
  for (Uint i=1; i<PE::Comm::instance().size(); ++i)
    recv_displs[i] = recv_displs[i-1] + recv_strides[i-1];
  recv.reset();
  recv.resize(recv_displs.back()+recv_strides.back());
  MPI_CHECK_RESULT(MPI_Alltoallv, ((void*)send_linear.begin(), &send_strides[0], &send_displs[0], MPI_PACKED, (void*)recv.begin(), &recv_strides[0], &recv_displs[0], MPI_PACKED, PE::Comm::instance().communicator()));
}

////////////////////////////////////////////////////////////////////////////////

void flex_all_to_all(const PE::Buffer& send, std::vector<int>& send_strides, PE::Buffer& recv, std::vector<int>& recv_strides)
{
  std::vector<int> send_displs(send_strides.size());
  if (send_strides.size()) send_displs[0] = 0;
  for (Uint i=1; i<send_strides.size(); ++i)
    send_displs[i] = send_displs[i-1] + send_strides[i-1];

  recv_strides.resize(PE::Comm::instance().size());
  std::vector<int> recv_displs(PE::Comm::instance().size());
  PE::Comm::instance().all_to_all(send_strides,recv_strides);
  if (recv_displs.size()) recv_displs[0] = 0;
  for (Uint i=1; i<PE::Comm::instance().size(); ++i)
    recv_displs[i] = recv_displs[i-1] + recv_strides[i-1];
  recv.reset();
  recv.resize(recv_displs.back()+recv_strides.back());
  MPI_CHECK_RESULT(MPI_Alltoallv, ((void*)send.begin(), &send_strides[0], &send_displs[0], MPI_PACKED, (void*)recv.begin(), &recv_strides[0], &recv_displs[0], MPI_PACKED, PE::Comm::instance().communicator()));
}

//////////////////////////////////////////////////////////////////////////////

void MeshPartitioner::migrate()
{
  if (PE::Comm::instance().is_active() == false)
    return;

  MeshAdaptor mesh_adaptor(*m_mesh);
  mesh_adaptor.prepare();
  mesh_adaptor.move_elements(m_elements_to_export);
  mesh_adaptor.finish();

}

//////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
