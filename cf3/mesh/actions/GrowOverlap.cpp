// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>
#include <map>

#include "common/Log.hpp"
#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/Buffer.hpp"
#include "common/PE/debug.hpp"


#include "mesh/Faces.hpp"
#include "mesh/Region.hpp"
#include "mesh/FaceCellConnectivity.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Manipulations.hpp"
#include "mesh/DynTable.hpp"
#include "mesh/Table.hpp"
#include "mesh/List.hpp"
#include "mesh/Geometry.hpp"
#include "mesh/MeshElements.hpp"

#include "mesh/Actions/GrowOverlap.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace Actions {

  using namespace common;
  using namespace common::PE;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < GrowOverlap, MeshTransformer, LibActions> GrowOverlap_Builder;

//////////////////////////////////////////////////////////////////////////////

template <typename T>
void my_all_gather(const std::vector<T>& send, std::vector<std::vector<T> >& recv)
{
  std::vector<int> strides;
  PE::Comm::instance().all_gather((int)send.size(),strides);
  std::vector<int> displs(strides.size());
  if (strides.size())
  {
    int sum_strides = strides[0];
    displs[0] = 0;
    for (Uint i=1; i<strides.size(); ++i)
    {
      displs[i] = displs[i-1] + strides[i-1];
      sum_strides += strides[i];
    }
    std::vector<T> recv_linear(sum_strides);
    MPI_CHECK_RESULT(MPI_Allgatherv, ((void*)&send[0], (int)send.size(), get_mpi_datatype<T>(), &recv_linear[0], &strides[0], &displs[0], get_mpi_datatype<T>(), PE::Comm::instance().communicator()));
    recv.resize(strides.size());
    for (Uint i=0; i<strides.size(); ++i)
    {
      recv[i].resize(strides[i]);
      for (Uint j=0; j<strides[i]; ++j)
      {
        recv[i][j]=recv_linear[displs[i]+j];
      }
    }
  }
  else
  {
    recv.resize(0);
  }
}

template <typename T>
void my_all_to_all(const std::vector<std::vector<T> >& send, std::vector<std::vector<T> >& recv)
{
  std::vector<int> send_strides(send.size());
  std::vector<int> send_displs(send.size());
  for (Uint i=0; i<send.size(); ++i)
    send_strides[i] = send[i].size();

  send_displs[0] = 0;
  for (Uint i=1; i<send.size(); ++i)
    send_displs[i] = send_displs[i-1] + send_strides[i-1];

  std::vector<T> send_linear(send_displs.back()+send_strides.back());
  for (Uint i=0; i<send.size(); ++i)
    for (Uint j=0; j<send[i].size(); ++j)
      send_linear[send_displs[i]+j] = send[i][j];

  std::vector<int> recv_strides(PE::Comm::instance().size());
  std::vector<int> recv_displs(PE::Comm::instance().size());
  PE::Comm::instance().all_to_all(send_strides,recv_strides);
  recv_displs[0] = 0;
  for (Uint i=1; i<PE::Comm::instance().size(); ++i)
    recv_displs[i] = recv_displs[i-1] + recv_strides[i-1];

  std::vector<T> recv_linear(recv_displs.back()+recv_strides.back());
  MPI_CHECK_RESULT(MPI_Alltoallv, (&send_linear[0], &send_strides[0], &send_displs[0], PE::get_mpi_datatype<Uint>(), &recv_linear[0], &recv_strides[0], &recv_displs[0], get_mpi_datatype<Uint>(), PE::Comm::instance().communicator()));

  recv.resize(recv_strides.size());
  for (Uint i=0; i<recv_strides.size(); ++i)
  {
    recv[i].resize(recv_strides[i]);
    for (Uint j=0; j<recv_strides[i]; ++j)
    {
      recv[i][j]=recv_linear[recv_displs[i]+j];
    }
  }
}

void my_all_to_all(const std::vector<PE::Buffer>& send, PE::Buffer& recv)
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

void my_all_to_all(const PE::Buffer& send, std::vector<int>& send_strides, PE::Buffer& recv, std::vector<int>& recv_strides)
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

////////////////////////////////////////////////////////////////////////////////

GrowOverlap::GrowOverlap( const std::string& name )
: MeshTransformer(name)
{

  properties()["brief"] = std::string("Grows the overlap layer of the mesh");
  std::string desc;
  desc =
      " Boundary nodes of one rank are communicated to other ranks.\n"
      " Each other rank then communicates all elements that are connected \n"
      " to these boundary nodes. \n"
      " Missing nodes are then also communicated to complete the elements";
  properties()["description"] = desc;
}

/////////////////////////////////////////////////////////////////////////////

void GrowOverlap::execute()
{

  Mesh& mesh = *m_mesh.lock();
  Geometry& nodes = mesh.geometry();

  const std::vector< boost::weak_ptr<Component> >& mesh_elements = mesh.elements().components();

  FaceCellConnectivity& face2cell = mesh.create_component<FaceCellConnectivity>("face2cell");
  face2cell.setup(mesh.topology());


  std::map<Uint,Uint> glb_node_2_loc_node;
  std::map<Uint,Uint>::iterator glb_node_not_found = glb_node_2_loc_node.end();
  for (Uint n=0; n<nodes.size(); ++n)
  {
    if ( glb_node_2_loc_node.find(nodes.glb_idx()[n]) == glb_node_not_found )
    {
      glb_node_2_loc_node[nodes.glb_idx()[n]] = n;
    }
    else
    {
      std::cout << PERank << "node glb idx " << nodes.glb_idx()[n] << " already exists..." << std::endl;
    }
  }

  std::map<Uint,Uint> glb_elem_2_loc_elem;
  std::map<Uint,Uint>::iterator glb_elem_not_found = glb_elem_2_loc_elem.end();
  for (Uint e=0; e<mesh.elements().size(); ++e)
  {
    Component::Ptr comp;
    Uint idx;

    boost::tie(comp,idx) = mesh.elements().location(e);
    if ( Elements::Ptr elements = comp->as_ptr<Elements>() )
    {
      if ( glb_elem_2_loc_elem.find(elements->glb_idx()[idx]) == glb_elem_not_found )
      {
        glb_elem_2_loc_elem[elements->glb_idx()[idx]] = e;
      }
      else
      {
        std::cout << PERank << "elem glb idx " << elements->glb_idx()[idx] << " already exists..." << std::endl;
      }
    }
  }

  std::set<Uint> bdry_nodes;
  for (Uint f=0; f<face2cell.size(); ++f)
  {
    cf3_assert(f < face2cell.is_bdry_face().size());
    if (face2cell.is_bdry_face()[f])
    {
      boost_foreach(const Uint node, face2cell.face_nodes(f))
          bdry_nodes.insert(nodes.glb_idx()[node]);
    }
  }
  boost_foreach (Faces& faces, find_components_recursively<Faces>(mesh.topology()))
  {
    boost_foreach (Connectivity::Row face_nodes, faces.node_connectivity().array())
    {
      boost_foreach(const Uint node, face_nodes)
      {
        bdry_nodes.insert(nodes.glb_idx()[node]);
      }
    }
  }
  mesh.remove_component(face2cell);

  // -----------------------------------------------------------------------------
  // SEARCH FOR CONNECTED ELEMENTS
  // in  : nodes                            std::vector<Uint>
  // out : buffer with packed elements      PE::Buffer(nodes)

  // COMMUNICATE NODES TO LOOK FOR

  std::vector<Uint> send_nodes; send_nodes.reserve(bdry_nodes.size());
  boost_foreach(const Uint n, bdry_nodes)
    send_nodes.push_back(n);
  std::vector<std::vector<Uint> > recv_nodes;
  my_all_gather(send_nodes,recv_nodes);



  // elem_idx_to_send[from_comp][to_proc][elem_idx]
  std::vector< std::vector < std::set<Uint> > > elem_ids_to_send(mesh_elements.size());
  for (Uint comp_idx=0; comp_idx<elem_ids_to_send.size(); ++comp_idx)
    elem_ids_to_send[comp_idx].resize(PE::Comm::instance().size());


  // storage for nodes that will need to be fetched after elements have been received
  std::set<Uint> new_ghost_nodes;

  for (Uint proc=0; proc<Comm::instance().size(); ++proc)
  {
    if (proc != Comm::instance().rank())
    {
      for (Uint n=0; n<recv_nodes[proc].size(); ++n)
      {
        Uint find_glb_node_idx = recv_nodes[proc][n];

        // +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
        if ( glb_node_2_loc_node.find(find_glb_node_idx) != glb_node_not_found)
        {
          Uint loc_idx = glb_node_2_loc_node[find_glb_node_idx];
          DynTable<Uint>::ConstRow connected_elements = nodes.glb_elem_connectivity()[loc_idx];
          boost_foreach ( const Uint glb_elem_idx, nodes.glb_elem_connectivity()[loc_idx] )
          {

            if ( glb_elem_2_loc_elem.find(glb_elem_idx) != glb_elem_not_found)
            {
              Uint unif_elem_idx = glb_elem_2_loc_elem[glb_elem_idx];

              Uint elem_comp_idx;
              Uint elem_idx;
              boost::tie(elem_comp_idx,elem_idx) = mesh.elements().location_idx(unif_elem_idx);

              if (mesh_elements[elem_comp_idx].lock()->as_type<Elements>().is_ghost(elem_idx) == false)
              {
                elem_ids_to_send[elem_comp_idx][proc].insert(elem_idx);
              }

            }

          }

        }
        // +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
      }
    }
  }

  std::vector<Uint> old_elem_size(mesh_elements.size());
  std::vector<Uint> new_elem_size(mesh_elements.size());

  for (Uint comp_idx=0; comp_idx<mesh_elements.size(); ++comp_idx)
  {
    if (Elements::Ptr elements_ptr = mesh_elements[comp_idx].lock()->as_ptr<Elements>())
    {
      Elements& elements = *elements_ptr;
      PackUnpackElements copy(elements);

      std::vector<PE::Buffer> elements_to_send(Comm::instance().size());
      PE::Buffer elements_to_recv;

      // Pack
      for (Uint to_proc = 0; to_proc<PE::Comm::instance().size(); ++to_proc)
      {
        boost_foreach(const Uint elem_idx, elem_ids_to_send[comp_idx][to_proc])
        {

          elements_to_send[to_proc] << elements.glb_idx()[elem_idx]
                                    << elements.rank()[elem_idx];

          boost_foreach(const Uint connected_node, elements.node_connectivity()[elem_idx])
              elements_to_send[to_proc] << nodes.glb_idx()[connected_node];

        }
      }

      // Communicate
      my_all_to_all(elements_to_send,elements_to_recv);

      // Save old_size
      old_elem_size[comp_idx] = elements.size();

      // Unpack
      while (elements_to_recv.more_to_unpack())
      {
        elements_to_recv >> copy;
      }

      copy.flush();

      new_elem_size[comp_idx] = elements.size();

      RemoveElements remove(elements);
      for (Uint e=old_elem_size[comp_idx]; e<new_elem_size[comp_idx]; ++e)
      {
        if ( glb_elem_2_loc_elem.find(elements.glb_idx()[e]) != glb_elem_not_found )
        {
          remove(e);
        }
      }
      remove.flush();
      new_elem_size[comp_idx] = elements.size();

      std::set<Uint>::iterator found_bdry_node;
      std::set<Uint>::iterator not_found = bdry_nodes.end();
      for (Uint e=old_elem_size[comp_idx]; e<new_elem_size[comp_idx]; ++e)
      {
        boost_foreach(const Uint connected_glb_node, elements.node_connectivity()[e])
        {
          if ( glb_node_2_loc_node.find(connected_glb_node) == glb_node_not_found)
            new_ghost_nodes.insert(connected_glb_node);
        }
      }


    }
    else
    {
      /// @todo case of non-Elements
    }
  }

  // -----------------------------------------------------------------------------
  // SEARCH FOR REQUESTED NODES
  // in  : requested nodes                std::vector<Uint>
  // out : buffer with packed nodes       PE::Buffer(nodes)

  // COMMUNICATE NODES TO LOOK FOR

  std::vector<Uint> request_nodes; request_nodes.reserve(new_ghost_nodes.size());
  boost_foreach(const Uint n, new_ghost_nodes)
      request_nodes.push_back(n);

  std::vector<std::vector<Uint> > recv_request_nodes;
  my_all_gather(request_nodes,recv_request_nodes);


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
        if ( glb_node_2_loc_node.find(find_glb_idx) != glb_node_not_found)
        {
          Uint loc_idx = glb_node_2_loc_node[find_glb_idx];
          //std::cout << PERank << "copying node " << glb_idx << " from loc " << loc_idx << std::flush;
          if (nodes.is_ghost(loc_idx) == false)
            nodes_to_send[proc] << copy_node(loc_idx,PackUnpackNodes::COPY);
        }
        // +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
      }
    }
  }

  // COMMUNICATE FOUND NODES BACK TO RANK THAT REQUESTED IT

  PE::Buffer received_nodes_buffer;
  my_all_to_all(nodes_to_send,received_nodes_buffer);

  // out: buffer containing requested nodes
  // -----------------------------------------------------------------------------

  // ADD GHOST NODES

  Uint old_nodes_size = nodes.size();
  PackUnpackNodes add_node(nodes);
  while (received_nodes_buffer.more_to_unpack())
    received_nodes_buffer >> add_node;
  add_node.flush();
  Uint new_nodes_size = nodes.size();

  // -----------------------------------------------------------------------------
  // REQUESTED GHOST-NODES HAVE NOW BEEN ADDED
  // -----------------------------------------------------------------------------

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
  for (Uint comp_idx=0; comp_idx<mesh_elements.size(); ++comp_idx)
  {
    if (Elements::Ptr elements_ptr = mesh_elements[comp_idx].lock()->as_ptr<Elements>())
    {
      Elements& elements = *elements_ptr;

      for (Uint e=old_elem_size[comp_idx]; e < new_elem_size[comp_idx]; ++e)
      {
        Connectivity::Row connected_nodes = elements.node_connectivity()[e];

        boost_foreach ( Uint& node, connected_nodes )
        {
          node = glb_to_loc[node];
        }

      }

    }

  }

  mesh.elements().reset();
  mesh.elements().update();
  mesh.update_statistics();

}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // mesh
} // cf3
