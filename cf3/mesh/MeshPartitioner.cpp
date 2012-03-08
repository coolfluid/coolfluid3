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
#include "mesh/Manipulations.hpp"
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
  options().add_option("nb_parts", m_nb_parts)
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
  //show_changes();
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

  options.add_option("mesh", URI())
      .description("Mesh to load balance");
}

//////////////////////////////////////////////////////////////////////

void MeshPartitioner::initialize(Mesh& mesh)
{
  m_mesh = Handle<Mesh>(mesh.handle<Component>());

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
  m_elements_to_export.resize(find_components_recursively<Elements>(mesh.topology()).size());
  for (Uint c=0; c<m_elements_to_export.size(); ++c)
    m_elements_to_export[c].resize(m_nb_parts);

  build_global_to_local_index(mesh);
  build_graph();
}

//////////////////////////////////////////////////////////////////////////////

void MeshPartitioner::build_global_to_local_index(Mesh& mesh)
{
  Dictionary& nodes = mesh.geometry_fields();

  m_lookup->add(nodes);
  boost_foreach ( Entities& elements, mesh.topology().elements_range() )
    m_lookup->add(elements);

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

  boost_foreach ( Entities& elements, mesh.topology().elements_range() )
  {
    m_nb_owned_obj += elements.size();
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
  boost_foreach ( Elements& elements, find_components_recursively<Elements>(mesh))
  {
    boost_foreach (Uint glb_idx, elements.glb_idx().array())
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

  boost_foreach(std::vector<std::vector<Uint> >& export_elems_from_region, m_elements_to_export)
    boost_foreach(std::vector<Uint>& export_elems_from_region_to_part, export_elems_from_region)
      nb_changes += export_elems_from_region_to_part.size();

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
      for (Uint comp=0; comp<m_elements_to_export.size(); ++comp)
      {
        std::string elements = m_lookup->components()[comp+1]->uri().path();
        for (Uint to_part=0; to_part<m_elements_to_export[comp].size(); ++to_part)
        {
          std::cout << "[" << PE::Comm::instance().rank() << "] export " << elements << " to part " << to_part << ":  ";
          for (Uint e=0; e<m_elements_to_export[comp][to_part].size(); ++e)
            std::cout << m_elements_to_export[comp][to_part][e] << " ";
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
    send_strides[i] = send[i].packed_size();

  if (send.size()) send_displs[0] = 0;
  for (Uint i=1; i<send.size(); ++i)
    send_displs[i] = send_displs[i-1] + send_strides[i-1];

  PE::Buffer send_linear;

  send_linear.resize(send_displs.back()+send_strides.back());
  for (Uint i=0; i<send.size(); ++i)
    send_linear.pack(send[i].buffer(),send[i].packed_size());

  std::vector<int> recv_strides(PE::Comm::instance().size());
  std::vector<int> recv_displs(PE::Comm::instance().size());
  PE::Comm::instance().all_to_all(send_strides,recv_strides);
  if (recv_displs.size()) recv_displs[0] = 0;
  for (Uint i=1; i<PE::Comm::instance().size(); ++i)
    recv_displs[i] = recv_displs[i-1] + recv_strides[i-1];
  recv.reset();
  recv.resize(recv_displs.back()+recv_strides.back());
  MPI_CHECK_RESULT(MPI_Alltoallv, ((void*)send_linear.buffer(), &send_strides[0], &send_displs[0], MPI_PACKED, (void*)recv.buffer(), &recv_strides[0], &recv_displs[0], MPI_PACKED, PE::Comm::instance().communicator()));
  recv.packed_size()=recv_displs.back()+recv_strides.back();
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
  MPI_CHECK_RESULT(MPI_Alltoallv, ((void*)send.buffer(), &send_strides[0], &send_displs[0], MPI_PACKED, (void*)recv.buffer(), &recv_strides[0], &recv_displs[0], MPI_PACKED, PE::Comm::instance().communicator()));
  recv.packed_size()=recv_displs.back()+recv_strides.back();
}

//////////////////////////////////////////////////////////////////////////////

void MeshPartitioner::migrate()
{
  if (PE::Comm::instance().is_active() == false)
    return;

  Uint nb_changes(0);
  boost_foreach(std::vector<Uint>& export_nodes_to_part, m_nodes_to_export)
    nb_changes += export_nodes_to_part.size();

  boost_foreach(std::vector<std::vector<Uint> >& export_elems_from_region, m_elements_to_export)
    boost_foreach(std::vector<Uint>& export_elems_from_region_to_part, export_elems_from_region)
      nb_changes += export_elems_from_region_to_part.size();

  Uint glb_changes;
  PE::Comm::instance().all_reduce( PE::plus(), &nb_changes,1,&glb_changes);

  if ( glb_changes == 0)
    return;


  Mesh& mesh = *m_mesh;
  Dictionary& nodes = mesh.geometry_fields();

  // ----------------------------------------------------------------------------
  // ----------------------------------------------------------------------------
  //                            MIGRATION ALGORITHM
  // ----------------------------------------------------------------------------
  // ----------------------------------------------------------------------------

  PackUnpackNodes node_manipulation(nodes);

  // -----------------------------------------------------------------------------
  // REMOVE GHOST NODES AND GHOST ELEMENTS

  for (Uint n=0; n<nodes.size(); ++n)
  {
    if (nodes.is_ghost(n))
      node_manipulation.remove(n);
  }

  // DONT FLUSH YET!!! node_manipulation.flush()

  boost_foreach(Elements& elements, find_components_recursively<Elements>(mesh.topology()) )
  {
    PackUnpackElements element_manipulation(elements);

    if (elements.rank().size() != elements.size())
      throw ValueNotFound(FromHere(),elements.uri().string()+" --> mismatch in element sizes (rank.size() = "+to_str(elements.rank().size())+" , elements.size() = "+to_str(elements.size())+")");
    for (Uint e=0; e<elements.size(); ++e)
    {
      if (elements.rank()[e] != PE::Comm::instance().rank())
        element_manipulation.remove(e);
    }
    /// @todo mechanism not to flush element_manipulation until during real migration
  }

  Comm::instance().barrier();
  CFdebug << "        * removed ghost elements and ghost nodes" << CFendl;

  // -----------------------------------------------------------------------------
  // SET NODE CONNECTIVITY TO GLOBAL NUMBERS BEFORE PARTITIONING

  const common::List<Uint>& global_node_indices = mesh.geometry_fields().glb_idx();
  boost_foreach (Entities& elements, mesh.topology().elements_range())
  {
    boost_foreach ( common::Table<Uint>::Row nodes, Handle<Elements>(elements.handle<Component>())->geometry_space().connectivity().array() )
    {
      boost_foreach ( Uint& node, nodes )
      {
        node = global_node_indices[node];
      }
    }
  }


  // -----------------------------------------------------------------------------
  // SEND ELEMENTS AND NODES FROM PARTITIONING ALGORITHM

  std::vector< Handle<Component> > mesh_element_comps = mesh.elements().components();

  PE::Buffer send_to_proc;  std::vector<int> send_strides(PE::Comm::instance().size());
  PE::Buffer recv_from_all; std::vector<int> recv_strides(PE::Comm::instance().size());

  // Move elements
  for(Uint i=0; i<mesh_element_comps.size(); ++i)
  {
    Elements& elements = dynamic_cast<Elements&>(*mesh_element_comps[i]);

    send_to_proc.reset();
    recv_from_all.reset();

    PackUnpackElements migrate_element(elements);
    std::vector<Uint> nb_elems_to_send(PE::Comm::instance().size());

    for (Uint r=0; r<PE::Comm::instance().size(); ++r)
    {
      Uint displs = send_to_proc.packed_size();
      for (Uint e=0; e<exported_elements()[i][r].size(); ++e)
        send_to_proc << migrate_element(exported_elements()[i][r][e],PackUnpackElements::MIGRATE);
      send_strides[r] = send_to_proc.packed_size() - displs;
    }

    flex_all_to_all(send_to_proc,send_strides,recv_from_all,recv_strides);

    while(recv_from_all.more_to_unpack())
      recv_from_all >> migrate_element;
    migrate_element.flush();
  }


  // Move nodes
   send_to_proc.reset();
   recv_from_all.reset();

   std::set<Uint> packed_nodes;
   for (Uint r=0; r<PE::Comm::instance().size(); ++r)
   {
     Uint displs = send_to_proc.packed_size();
     for (Uint n=0; n<exported_nodes()[r].size(); ++n)
     {
       send_to_proc << node_manipulation(exported_nodes()[r][n],PackUnpackNodes::MIGRATE);
     }
     send_strides[r] = send_to_proc.packed_size() - displs;
   }

  // STILL DONT FLUSH!!! node_manipulation.flush();

   flex_all_to_all(send_to_proc,send_strides,recv_from_all,recv_strides);


   while (recv_from_all.more_to_unpack())
    recv_from_all >> node_manipulation;

   // FINALLY FLUSH NODES
   node_manipulation.flush();

   nodes.check_sanity();


   // -----------------------------------------------------------------------------
   // MARK EVERYTHING AS OWNED

   for (Uint n=0; n<nodes.size(); ++n)
     nodes.rank()[n] = PE::Comm::instance().rank();

   boost_foreach(Entities& elements, mesh.topology().elements_range())
   {
     for (Uint e=0; e<elements.size(); ++e)
       elements.rank()[e] = PE::Comm::instance().rank();
   }


  // -----------------------------------------------------------------------------
  // ELEMENTS AND NODES HAVE BEEN MOVED
  // -----------------------------------------------------------------------------

   Comm::instance().barrier();
   CFdebug << "        * elements and nodes migrated, request ghost nodes" << CFendl;

  // -----------------------------------------------------------------------------
  // COLLECT GHOST-NODES TO LOOK FOR ON OTHER PROCESSORS

  std::set<Uint> owned_nodes;
  for (Uint n=0; n<nodes.size(); ++n)
    owned_nodes.insert(nodes.glb_idx()[n]);

  std::set<Uint> ghost_nodes;
  boost_foreach(const Elements& elements, find_components_recursively<Elements>(mesh.topology()))
  {
    boost_foreach(Connectivity::ConstRow connected_nodes, elements.geometry_space().connectivity().array())
    {
      boost_foreach(const Uint node, connected_nodes)
      {
        if (owned_nodes.find(node) == owned_nodes.end())
          ghost_nodes.insert(node);
      }
    }
  }

  std::vector<Uint> request_nodes;  request_nodes.reserve(ghost_nodes.size());
  boost_foreach(const Uint node, ghost_nodes)
    request_nodes.push_back(node);


  // -----------------------------------------------------------------------------
  // SEARCH FOR REQUESTED NODES
  // in  : requested nodes                std::vector<Uint>
  // out : buffer with packed nodes       PE::Buffer(nodes)

  // COMMUNICATE NODES TO LOOK FOR

  std::vector<std::vector<Uint> > recv_request_nodes;
  Comm::instance().all_gather(request_nodes,recv_request_nodes);

  PackUnpackNodes copy_node(nodes);
  std::vector<PE::Buffer> nodes_to_send(Comm::instance().size());
  for (Uint proc=0; proc<Comm::instance().size(); ++proc)
  {
    if (proc != Comm::instance().rank())
    {

      for (Uint n=0; n<recv_request_nodes[proc].size(); ++n)
      {
        Uint find_glb_idx = recv_request_nodes[proc][n];

        // +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
        /// @todo THIS ALGORITHM HAS TO BE IMPROVED (BRUTE FORCE)
        Uint loc_idx=0;
        bool found=false;
        boost_foreach(const Uint glb_idx, nodes.glb_idx().array())
        {

          cf3_assert(loc_idx < nodes.size());
          if (glb_idx == find_glb_idx)
          {
            //std::cout << PERank << "copying node " << glb_idx << " from loc " << loc_idx << std::flush;
            nodes_to_send[proc] << copy_node(loc_idx,PackUnpackNodes::COPY);

            break;
          }
          ++loc_idx;
        }
        // +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
      }
    }
  }

  // COMMUNICATE FOUND NODES BACK TO RANK THAT REQUESTED IT

  PE::Buffer received_nodes_buffer;
  flex_all_to_all(nodes_to_send,received_nodes_buffer);

  // out: buffer containing requested nodes
  // -----------------------------------------------------------------------------

  // ADD GHOST NODES

  PackUnpackNodes add_node(nodes);
  while (received_nodes_buffer.more_to_unpack())
    received_nodes_buffer >> add_node;
  add_node.flush();

  // -----------------------------------------------------------------------------
  // REQUESTED GHOST-NODES HAVE NOW BEEN ADDED
  // -----------------------------------------------------------------------------

  Comm::instance().barrier();
  CFdebug << "        * requested ghost nodes added" << CFendl;

  // -----------------------------------------------------------------------------
  // FIX NODE CONNECTIVITY
  std::map<Uint,Uint> glb_to_loc;
  std::map<Uint,Uint>::iterator it;
  bool inserted;
  for (Uint n=0; n<nodes.size(); ++n)
  {
    boost::tie(it,inserted) = glb_to_loc.insert(std::make_pair(nodes.glb_idx()[n],n));
    if (! inserted)
      throw ValueExists(FromHere(), std::string(nodes.is_ghost(n)? "ghost " : "" ) + "node["+to_str(n)+"] with glb_idx "+to_str(nodes.glb_idx()[n])+" already exists as "+to_str(glb_to_loc[n]));
  }
  boost_foreach (Entities& elements, mesh.topology().elements_range())
  {
    boost_foreach ( common::Table<Uint>::Row nodes, Handle<Elements>(elements.handle<Component>())->geometry_space().connectivity().array() )
    {
      boost_foreach ( Uint& node, nodes )
      {
        node = glb_to_loc[node];
      }
    }
  }

  // -----------------------------------------------------------------------------
  // MESH IS NOW COMPLETELY LOAD BALANCED WITHOUT OVERLAP
  // -----------------------------------------------------------------------------

  mesh.update_statistics();
  mesh.elements().reset();
  mesh.elements().update();

  Comm::instance().barrier();
  CFdebug << "        * migration complete" << CFendl;

}

//////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
