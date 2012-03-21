// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Log.hpp"
#include "common/FindComponents.hpp"
#include "common/Map.hpp"
#include "common/PropertyList.hpp"

#include "common/PE/debug.hpp"

#include "math/Consts.hpp"

#include "mesh/MeshAdaptor.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Space.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshElements.hpp"
#include "mesh/FaceCellConnectivity.hpp"

namespace cf3 {
namespace mesh {

using namespace common;
using namespace common::PE;
using namespace math::Consts;

////////////////////////////////////////////////////////////////////////////////

PackedElement::PackedElement(const mesh::Mesh& mesh) : m_mesh(mesh)
{
  m_connectivity.resize( m_mesh.dictionaries().size() );
}

////////////////////////////////////////////////////////////////////////////////

PackedElement::PackedElement(const mesh::Mesh& mesh, const Uint entities_idx, const Uint elem_loc_idx) : m_mesh(mesh)
{
  cf3_assert(mesh.dictionaries().size());
  m_connectivity.resize( mesh.dictionaries().size() );
  for (Uint dict_idx=0; dict_idx<m_connectivity.size(); ++dict_idx)
    m_connectivity[dict_idx].clear();

  m_entities_idx = entities_idx;
  m_loc_idx = elem_loc_idx;
  cf3_assert(m_entities_idx < mesh.elements().size());
  const Handle<Entities>& entities = mesh.elements()[entities_idx];
  cf3_assert(elem_loc_idx < entities->size());
  m_glb_idx = entities->glb_idx()[m_loc_idx];
  m_rank = entities->rank()[m_loc_idx];
  boost_foreach (const Handle<Space>& space, entities->spaces())
  {
    cf3_assert(space);
    const Uint nb_nodes = space->shape_function().nb_nodes();
    cf3_assert_desc(to_str(space->dict_idx())+"<"+to_str(m_connectivity.size()),space->dict_idx() < m_connectivity.size());
    m_connectivity[space->dict_idx()].resize(nb_nodes);
    for (Uint node=0; node<nb_nodes; ++node)
    {
      cf3_assert(m_loc_idx < space->connectivity().size());
      cf3_assert(node<(space->connectivity()[m_loc_idx].size()));
      m_connectivity[space->dict_idx()][node] = space->connectivity()[m_loc_idx][node];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void PackedElement::unpack(PE::Buffer& buf)
{
  Uint nb_spaces;
  buf >> m_entities_idx >> m_loc_idx >> m_glb_idx >> m_rank;
//  std::cout << PERank << "                      m_entities_idx = " << m_entities_idx << std::endl;
//  std::cout << PERank << "                      m_loc_idx = " << m_loc_idx << std::endl;
//  std::cout << PERank << "                      m_glb_idx = " << m_glb_idx << std::endl;
//  std::cout << PERank << "                      m_rank = " << m_rank << std::endl;
//  std::cout << PERank << "                      nb_spaces = " << nb_spaces << std::endl;
  cf3_assert(m_connectivity.size() == m_mesh.dictionaries().size());
  for (Uint dict_idx=0; dict_idx<m_mesh.dictionaries().size(); ++dict_idx)
  {
    buf >> m_connectivity[dict_idx];
  }
//  std::cout << PERank << "unpacked element    glb_idx = " << m_glb_idx << "\t    rank = " << m_rank << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void PackedElement::pack(PE::Buffer& buf)
{
  buf << m_entities_idx << m_loc_idx << m_glb_idx << m_rank;
  for (Uint dict_idx=0; dict_idx<m_mesh.dictionaries().size(); ++dict_idx)
  {
    buf << m_connectivity[dict_idx];
  }
//  std::cout << PERank << "packed element    glb_idx = " << m_glb_idx << "\t    rank = " << m_rank << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

PackedNode::PackedNode(const mesh::Mesh& mesh, const Uint dict_idx, const Uint node_loc_idx): m_mesh(mesh)
{
  m_dict_idx = dict_idx;
  cf3_assert(m_dict_idx<mesh.dictionaries().size());
  const Dictionary& dict = *mesh.dictionaries()[m_dict_idx];
  m_loc_idx = node_loc_idx;
  cf3_assert(m_loc_idx < dict.glb_idx().size());
  m_glb_idx = dict.glb_idx()[m_loc_idx];
  cf3_assert(m_loc_idx < dict.rank().size());
  m_rank = dict.rank()[m_loc_idx];
  m_field_values.resize(dict.fields().size());
  for (Uint fields_idx=0; fields_idx<m_field_values.size(); ++fields_idx)
  {
    Field& field = *dict.fields()[fields_idx];
    m_field_values[fields_idx].resize(field.row_size());
    cf3_assert(m_loc_idx < field.size());
    for (Uint var=0; var<field.row_size(); ++var)
    {
      m_field_values[fields_idx][var] = field[m_loc_idx][var];
    }
  }
//  std::cout << PERank << "packed node    glb_idx = " << m_glb_idx << "\t    rank = " << m_rank << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void PackedNode::unpack(PE::Buffer& buf)
{
  Uint nb_fields;
  buf >> m_dict_idx >> m_loc_idx >> m_glb_idx >> m_rank >> nb_fields;
  m_field_values.resize(nb_fields);
  for (Uint fields_idx=0; fields_idx<nb_fields; ++fields_idx)
  {
    buf >> m_field_values[fields_idx];
  }
//  std::cout << PERank << "unpacked node    glb_idx = " << m_glb_idx << "\t    rank = " << m_rank << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void PackedNode::pack(PE::Buffer& buf)
{
  buf << m_dict_idx << m_loc_idx << m_glb_idx << m_rank << (Uint) m_field_values.size();
  for (Uint fields_idx=0; fields_idx<m_field_values.size(); ++fields_idx)
  {
    buf << m_field_values[fields_idx];
  }
  // std::cout << PERank << "packed node    glb_idx = " << m_glb_idx << "\t    rank = " << m_rank << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

MeshAdaptor::MeshAdaptor(mesh::Mesh &mesh)
{
  is_node_connectivity_global = false;
  node_glb_to_loc_needs_rebuild = false;
  node_elem_connectivity_needs_rebuild = false;
  elem_flush_required = false;
  node_flush_required = false;

  m_mesh = mesh.handle<Mesh>();
}

////////////////////////////////////////////////////////////////////////////////

MeshAdaptor::~MeshAdaptor()
{
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::prepare()
{
//  std::cout << PERank << "preparing mesh_adaptor" << std::endl;
//  make_element_node_connectivity_global();
//  std::cout << PERank << "  - node_connectivity_global" << std::endl;
  create_element_buffers();
//  std::cout << PERank << "  - create_element_buffers" << std::endl;
  create_node_buffers();
//  std::cout << PERank << "  - create_node_buffers" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::finish()
{
  flush_nodes();
  flush_elements();

  restore_element_node_connectivity();
  rebuild_node_glb_to_loc_map();
  rebuild_node_to_element_connectivity();

  cf3_assert( ! is_node_connectivity_global );
  cf3_assert( ! node_glb_to_loc_needs_rebuild );
  cf3_assert( ! node_elem_connectivity_needs_rebuild );

  m_mesh->update_statistics();
  m_mesh->update_structures();
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::create_element_buffers()
{
  element_glb_idx.clear();
  element_rank.clear();
  element_connected_nodes.clear();

  element_glb_idx.resize(m_mesh->elements().size());
  element_rank.resize(m_mesh->elements().size());
  element_connected_nodes.resize(m_mesh->elements().size());

  added_elements.resize(m_mesh->elements().size());
  added_elements.clear();

  for (Uint ent=0; ent<m_mesh->elements().size(); ++ent)
  {
    const Handle<Entities>& elements = m_mesh->elements()[ent];
    if (is_not_null(elements))
    {
      element_glb_idx[ent] = elements->glb_idx().create_buffer_ptr();
      element_rank[ent]    = elements->rank().create_buffer_ptr();
      element_connected_nodes[ent].resize(elements->spaces().size());
      for (Uint space_idx=0; space_idx < elements->spaces().size(); ++space_idx )
        element_connected_nodes[ent][space_idx] = elements->spaces()[space_idx]->connectivity().create_buffer_ptr();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::create_node_buffers()
{
  node_glb_idx.clear();
  node_rank.clear();
  node_field_values.clear();

  node_glb_idx.resize(m_mesh->dictionaries().size());
  node_rank.resize(m_mesh->dictionaries().size());
  node_field_values.resize(m_mesh->dictionaries().size());

  added_nodes.resize(m_mesh->dictionaries().size());
  added_nodes.clear();

  for (Uint dict_idx=0; dict_idx<m_mesh->dictionaries().size(); ++dict_idx)
  {
    const Handle<Dictionary>& dict = m_mesh->dictionaries()[dict_idx];
    if (is_not_null(dict))
    {
      node_glb_idx[dict_idx] = dict->glb_idx().create_buffer_ptr();
      node_rank[dict_idx]    = dict->rank().create_buffer_ptr();
      node_field_values[dict_idx].resize(dict->fields().size());
      for (Uint fields_idx=0; fields_idx < dict->fields().size(); ++fields_idx )
        node_field_values[dict_idx][fields_idx] = dict->fields()[fields_idx]->create_buffer_ptr();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::make_element_node_connectivity_global()
{
  if (!is_node_connectivity_global)
  {
    CFdebug << "MeshAdaptor: make element-node connectivity global" << CFendl;
    boost_foreach (Entities& elements, find_components_recursively<Entities>(*m_mesh))
    {
      boost_foreach(const Handle<Space>& space, elements.spaces())
      {
        boost_foreach ( Connectivity::Row nodes, space->connectivity().array() )
        {
          boost_foreach ( Uint& node, nodes )
          {
            node = space->dict().glb_idx()[node];
          }
        }
      }
    }
  }
  is_node_connectivity_global = true;
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::restore_element_node_connectivity()
{
  if (is_node_connectivity_global)
  {
    CFdebug << "MeshAdaptor: make element-node connectivity local" << CFendl;
    rebuild_node_glb_to_loc_map();
    boost_foreach (Entities& elements, find_components_recursively<Entities>(*m_mesh))
    {
      boost_foreach(const Handle<Space>& space, elements.spaces())
      {
        const common::Map<boost::uint64_t,Uint>& glb_to_loc = space->dict().glb_to_loc();
        boost_foreach ( Connectivity::Row nodes, space->connectivity().array() )
        {
          boost_foreach ( Uint& node, nodes )
          {
            node = glb_to_loc[node];
            cf3_assert( node < space->dict().size() );
          }
        }
      }
    }
  }
  is_node_connectivity_global = false;
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::add_element(const PackedElement& packed_element)
{
  bool not_added_yet = added_elements[packed_element.entities_idx()].insert(packed_element.glb_idx()).second;
  if (not_added_yet)
  {
//    std::cout << PERank << " adding element " << packed_element.glb_idx() << std::endl;
    element_glb_idx[packed_element.entities_idx()]->add_row(packed_element.glb_idx());
    element_rank[packed_element.entities_idx()]->add_row(packed_element.rank());
    for (Uint space_idx=0; space_idx<element_connected_nodes[packed_element.entities_idx()].size(); ++space_idx)
    {
      cf3_assert(packed_element.connectivity()[space_idx].size() == element_connected_nodes[packed_element.entities_idx()][space_idx]->get_appointed().shape()[1]);
      element_connected_nodes[packed_element.entities_idx()][space_idx]->add_row(packed_element.connectivity()[space_idx]);
    }
    elem_flush_required = true;
  }
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::remove_element(const PackedElement& packed_element)
{
  remove_element(packed_element.entities_idx(),packed_element.loc_idx());
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::remove_element(const Uint entities_idx, const Uint elem_loc_idx)
{
//  std::cout << PERank << " removing element " << elem_loc_idx << std::endl;
  cf3_assert_desc("Forgot to call MeshAdaptor::create_element_buffers()",element_glb_idx.size());
  cf3_assert(entities_idx< element_glb_idx.size() );
  cf3_assert(elem_loc_idx < element_glb_idx[entities_idx]->total_allocated());
  element_glb_idx[entities_idx]->rm_row(elem_loc_idx);
  element_rank[entities_idx]->rm_row(elem_loc_idx);
  for (Uint space_idx=0; space_idx<element_connected_nodes[entities_idx].size(); ++space_idx)
    element_connected_nodes[entities_idx][space_idx]->rm_row(elem_loc_idx);
  added_elements[entities_idx].erase(m_mesh->elements()[entities_idx]->glb_idx()[elem_loc_idx]);
  elem_flush_required = true;
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::add_node(const PackedNode& packed_node)
{
  bool not_added_yet = added_nodes[packed_node.dict_idx()].insert(packed_node.glb_idx()).second;
  if (not_added_yet)
  {
//    std::cout << PERank << " adding node " << packed_node.glb_idx() << std::endl;
    cf3_assert_desc("Forgot to call MeshAdaptor::create_element_buffers()",node_glb_idx.size());
    node_glb_idx[packed_node.dict_idx()]->add_row(packed_node.glb_idx());
    node_rank[packed_node.dict_idx()]->add_row(packed_node.rank());
    for (Uint fields_idx=0; fields_idx<node_field_values[packed_node.dict_idx()].size(); ++fields_idx)
    {
      cf3_assert(packed_node.field_values()[fields_idx].size() == node_field_values[packed_node.dict_idx()][fields_idx]->get_appointed().shape()[1]);
      node_field_values[packed_node.dict_idx()][fields_idx]->add_row(packed_node.field_values()[fields_idx]);
    }
    node_flush_required = true;
  }
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::remove_node(const PackedNode& packed_node)
{
  remove_node(packed_node.dict_idx(),packed_node.loc_idx());
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::remove_node(const Uint dict_idx, const Uint node_loc_idx)
{
  cf3_assert_desc("Forgot to call MeshAdaptor::create_node_buffers()",node_glb_idx.size());
  cf3_assert(dict_idx< node_glb_idx.size() );
  cf3_assert(node_loc_idx < node_glb_idx[dict_idx]->total_allocated());
  node_glb_idx[dict_idx]->rm_row(node_loc_idx);
  node_rank[dict_idx]->rm_row(node_loc_idx);
  for (Uint fields_idx=0; fields_idx<node_field_values[dict_idx].size(); ++fields_idx)
    node_field_values[dict_idx][fields_idx]->rm_row(node_loc_idx);
  added_nodes[dict_idx].erase(m_mesh->dictionaries()[dict_idx]->glb_idx()[node_loc_idx]);
  node_flush_required = true;
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::flush_elements()
{
  if (elem_flush_required)
  {
    CFdebug << "MeshAdaptor: flushing elements" << CFendl;
    for (Uint c=0; c<m_mesh->elements().size(); ++c)
    {
      if (element_glb_idx[c])
        element_glb_idx[c]->flush();
      if (element_rank[c])
        element_rank[c]->flush();
      for (Uint s=0; s<element_connected_nodes[c].size(); ++s)
      {
        if (element_connected_nodes[c][s])
          element_connected_nodes[c][s]->flush();
      }
    }
    added_elements.clear();
    elem_flush_required = false;
    node_elem_connectivity_needs_rebuild = true;
  }
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::flush_nodes()
{
  if (node_flush_required)
  {
    CFdebug << "MeshAdaptor: flushing nodes" << CFendl;
    for (Uint c=0; c<m_mesh->dictionaries().size(); ++c)
    {
      if (node_glb_idx[c])
        node_glb_idx[c]->flush();
      if (node_rank[c])
        node_rank[c]->flush();
      for (Uint f=0; f<node_field_values[c].size(); ++f)
      {
        if (node_field_values[c][f])
          node_field_values[c][f]->flush();
      }
      added_nodes.clear();
    }
    node_flush_required = false;
    node_elem_connectivity_needs_rebuild = true;
    node_glb_to_loc_needs_rebuild = true;
  }
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::remove_ghost_elements()
{
  for (Uint entities_idx=0; entities_idx<m_mesh->elements().size(); entities_idx++)
  {
    const Entities& elements = *m_mesh->elements()[entities_idx];
    const Uint nb_elems = elements.size();
    for (Uint elem=0; elem<nb_elems; ++elem)
    {
      if (elements.is_ghost(elem))
        remove_element(entities_idx,elem);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::remove_ghost_nodes()
{
  for (Uint dict_idx=0; dict_idx<m_mesh->dictionaries().size(); dict_idx++)
  {
    const Dictionary& dict = *m_mesh->dictionaries()[dict_idx];
    for (Uint node=0; node<dict.size(); ++node)
    {
      if (dict.is_ghost(node))
        remove_node(dict_idx,node);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::find_nodes_to_export(const std::vector< std::vector< std::vector<Uint> > >& exported_elements_loc_id,
                                       std::vector< std::vector< std::vector<Uint> > >& exported_nodes_loc_id)
{
  const Uint nb_dicts = m_mesh->dictionaries().size();

  // a change-set of nodes to send
  std::vector< std::vector< std::set<Uint> > > nodes_to_send(PE::Comm::instance().size(),
                                                             std::vector< std::set<Uint> >(nb_dicts));

  if (is_node_connectivity_global)
  {
    rebuild_node_glb_to_loc_map();
  }

  for (Uint pid=0; pid<PE::Comm::instance().size(); ++pid)
  {
    cf3_assert(exported_elements_loc_id[pid].size() == m_mesh->elements().size());
    for (Uint entities_idx=0; entities_idx<m_mesh->elements().size(); ++entities_idx)
    {
      Entities& entities = *m_mesh->elements()[entities_idx];

      boost_foreach (const Uint loc_elem_idx, exported_elements_loc_id[pid][entities_idx])
      {
        // Collect nodes that participate in communication
        boost_foreach (const Handle<Space>& space, entities.spaces())
        {
          const Dictionary& dict = space->dict();

          const Uint dict_idx = space->dict_idx();
          cf3_assert(dict_idx < m_mesh->dictionaries().size());

          if (is_node_connectivity_global)
          {
            cf3_assert(dict.glb_to_loc().size());
            boost_foreach (const Uint glb_node, space->connectivity()[loc_elem_idx])
            {
              cf3_assert(dict.glb_to_loc().exists(glb_node));
              nodes_to_send[pid][dict_idx].insert( dict.glb_to_loc()[glb_node] );
            }
          }
          else
          {
            boost_foreach (const Uint loc_node, space->connectivity()[loc_elem_idx])
            {
              nodes_to_send[pid][dict_idx].insert( loc_node );
            }
          }
        }
      }
    }
  }

  exported_nodes_loc_id.resize(PE::Comm::instance().size(),
                               std::vector< std::vector<Uint> >(nb_dicts));
  for (Uint pid=0; pid<PE::Comm::instance().size(); ++pid)
  {
    for (Uint dict_idx=0; dict_idx<nb_dicts; ++dict_idx)
    {
      Dictionary& dict = *m_mesh->dictionaries()[dict_idx];
      exported_nodes_loc_id[pid][dict_idx].clear();
      exported_nodes_loc_id[pid][dict_idx].reserve(nodes_to_send[pid][dict_idx].size());
      boost_foreach (Uint loc_node, nodes_to_send[pid][dict_idx])
      {
        exported_nodes_loc_id[pid][dict_idx].push_back( loc_node );
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::send_elements(const std::vector< std::vector< std::vector<Uint> > >&      exported_elements_loc_id,
                                std::vector< std::vector< std::vector<boost::uint64_t> > >& imported_elements_glb_id)
{
  CFdebug << "MeshAdaptor: send elements" << CFendl;

  cf3_assert(exported_elements_loc_id.size() == PE::Comm::instance().size());
  const Uint nb_dicts = m_mesh->dictionaries().size();
  const Uint nb_entities = m_mesh->elements().size();
  for (Uint dict_idx=0; dict_idx<nb_dicts; ++dict_idx)
  {
    Dictionary& dict = *m_mesh->dictionaries()[dict_idx];
    cf3_assert(dict.glb_to_loc().size() == dict.size());
  }

  // Declaration of send/receive buffers
  PE::Buffer send_buffer, receive_buffer;

  // Element-node connectivity tables must be GLOBAL
  make_element_node_connectivity_global();

  // 1) Sending elements, and building nodes_to_send change set
  for (Uint pid=0; pid<PE::Comm::instance().size(); ++pid)
  {
    // Mark in the buffer that following will be sent to a new processor
    send_buffer.mark_pid_start();

    cf3_assert(exported_elements_loc_id[pid].size() == m_mesh->elements().size());
    for (Uint entities_idx=0; entities_idx<m_mesh->elements().size(); ++entities_idx)
    {

      Entities& entities = *m_mesh->elements()[entities_idx];

      boost_foreach (const Uint loc_elem_idx, exported_elements_loc_id[pid][entities_idx])
      {
        // Pack element in buffer to send
        PackedElement packed_elem(*m_mesh, entities_idx, loc_elem_idx );
        send_buffer << packed_elem;
      }
    }
  }

  ////PECheckArrivePoint(100,"Send/receive elements");

  // Send/Receive the elements.
  send_buffer.all_to_all(receive_buffer);

  // 2) Add the elements

  std::set< boost::uint64_t > mesh_elems;
  boost_foreach (const Handle<Entities>& entities, m_mesh->elements())
  {
    boost_foreach (const boost::uint64_t glb_elem, entities->glb_idx().array())
    {
      mesh_elems.insert(glb_elem);
    }
  }

  // Unpack elements from the receive_buffer on the receiving side
  std::vector< std::vector< std::set<boost::uint64_t> > > received_glb_elements_pid(PE::Comm::instance().size(), std::vector< std::set<boost::uint64_t> >(m_mesh->elements().size()));

  // Scope this
  {
    PackedElement unpacked_elem(*m_mesh);
    Uint recv_pid=0;
    cf3_assert(receive_buffer.size() == receive_buffer.displs().back()+receive_buffer.strides().back());
    while (receive_buffer.more_to_unpack())
    {
      cf3_assert(receive_buffer.displs().size() == PE::Comm::instance().size());
      cf3_assert(receive_buffer.strides().size() == PE::Comm::instance().size());
      cf3_assert(recv_pid < PE::Comm::instance().size());
      cf3_assert(receive_buffer.unpacked_idx() <= receive_buffer.displs()[recv_pid] + receive_buffer.strides()[recv_pid]);
      if (receive_buffer.unpacked_idx() == receive_buffer.displs()[recv_pid] + receive_buffer.strides()[recv_pid])
      {
        do { ++recv_pid; } while(receive_buffer.strides()[recv_pid]==0);
      }
      cf3_assert(receive_buffer.strides().size() == PE::Comm::instance().size());
      cf3_assert(receive_buffer.displs().size()  == PE::Comm::instance().size());
      cf3_assert(recv_pid<receive_buffer.strides().size());
      receive_buffer >> unpacked_elem;

      received_glb_elements_pid[recv_pid][unpacked_elem.entities_idx()].insert( unpacked_elem.glb_idx() );

      if (mesh_elems.count(unpacked_elem.glb_idx()) == 0)
        add_element(unpacked_elem);
    }
  }

  // Fill imported_elements_glb_id
  imported_elements_glb_id.resize(PE::Comm::instance().size(), std::vector< std::vector<boost::uint64_t> >(nb_entities));
  for (Uint pid=0; pid<PE::Comm::instance().size(); ++pid)
  {
    for (Uint entities_idx=0; entities_idx<nb_entities; ++entities_idx)
    {
      imported_elements_glb_id[pid][entities_idx].clear();
      imported_elements_glb_id[pid][entities_idx].reserve(received_glb_elements_pid[pid][entities_idx].size());
      boost_foreach (const boost::uint64_t& glb_elem, received_glb_elements_pid[pid][entities_idx])
      {
        imported_elements_glb_id[pid][entities_idx].push_back(glb_elem);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::send_nodes(const std::vector< std::vector< std::vector<Uint> > >&      exported_nodes_loc_id,
                             std::vector< std::vector< std::vector<boost::uint64_t> > >& imported_nodes_glb_id)
{
  CFdebug << "MeshAdaptor: send nodes" << CFendl;

  const Uint nb_dicts = m_mesh->dictionaries().size();
  for (Uint dict_idx=0; dict_idx<nb_dicts; ++dict_idx)
  {
    Dictionary& dict = *m_mesh->dictionaries()[dict_idx];
    cf3_assert(dict.glb_to_loc().size() == dict.size());
  }

  // Declaration of send/receive buffers
  PE::Buffer send_buffer, receive_buffer;

  // 3) Send nodes
  // Prepare send-buffer to be used again, now for nodes

  for (Uint pid=0; pid<PE::Comm::instance().size(); ++pid)
  {
    send_buffer.mark_pid_start();
    for (Uint dict_idx=0; dict_idx<nb_dicts; ++dict_idx)
    {
      Dictionary& dict = *m_mesh->dictionaries()[dict_idx];
      boost_foreach (Uint loc_node, exported_nodes_loc_id[pid][dict_idx])
      {
        PackedNode packed_node(*m_mesh,dict_idx,loc_node);
        send_buffer << packed_node;
      }
    }
  }

  ////PECheckArrivePoint(100,"nodes packed");

  // Send/Receive buffers
  send_buffer.all_to_all(receive_buffer);

  ////PECheckArrivePoint(100,"nodes sent/received");

  // 4) Add nodes on receiving side
  std::vector< std::vector<std::set<boost::uint64_t> > > received_glb_nodes_pid(PE::Comm::instance().size(),std::vector<std::set<boost::uint64_t> >(nb_dicts));
  // Scope this
  {
    PackedNode unpacked_node(*m_mesh);
    Uint recv_pid=0;
    while (receive_buffer.more_to_unpack())
    {
      cf3_assert(receive_buffer.unpacked_idx() <= receive_buffer.displs()[recv_pid] + receive_buffer.strides()[recv_pid]);
      if (receive_buffer.unpacked_idx() == receive_buffer.displs()[recv_pid] + receive_buffer.strides()[recv_pid])
      {
        do { ++recv_pid; } while(receive_buffer.strides()[recv_pid]==0);
      }
      receive_buffer >> unpacked_node;

      received_glb_nodes_pid[recv_pid][unpacked_node.dict_idx()].insert( unpacked_node.glb_idx() );

      // Component to check if a node is already existing. If so, the unpacked node doesn't need to be added anymore
      const common::Map<boost::uint64_t,Uint>& glb_to_loc = m_mesh->dictionaries()[unpacked_node.dict_idx()]->glb_to_loc();
      if (!glb_to_loc.exists(unpacked_node.glb_idx()))
      {
        add_node(unpacked_node);
      }
    }
  }

  ////PECheckArrivePoint(100,"nodes added");

  // Fill imported_nodes_glb_id
  imported_nodes_glb_id.resize(PE::Comm::instance().size(), std::vector< std::vector<boost::uint64_t> >(nb_dicts));
  for (Uint pid=0; pid<PE::Comm::instance().size(); ++pid)
  {
    for (Uint dict_idx=0; dict_idx<nb_dicts; ++dict_idx)
    {
      imported_nodes_glb_id[pid][dict_idx].clear();
      imported_nodes_glb_id[pid][dict_idx].reserve(received_glb_nodes_pid[pid][dict_idx].size());
      boost_foreach (const boost::uint64_t& glb_node, received_glb_nodes_pid[pid][dict_idx])
      {
        imported_nodes_glb_id[pid][dict_idx].push_back(glb_node);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::rebuild_node_glb_to_loc_map()
{
  if (node_glb_to_loc_needs_rebuild)
  {
    CFdebug << "MeshAdaptor: rebuild glb_to_loc maps in dictionaries" << CFendl;
    for (Uint dict_idx=0; dict_idx<m_mesh->dictionaries().size(); ++dict_idx)
    {
      Dictionary& dict = *m_mesh->dictionaries()[dict_idx];
      dict.rebuild_map_glb_to_loc();
    }
    node_glb_to_loc_needs_rebuild = false;
  }
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::rebuild_node_to_element_connectivity()
{
  if (node_elem_connectivity_needs_rebuild)
  {
    restore_element_node_connectivity();
    CFdebug << "MeshAdaptor: rebuild node-element connectivity in dictionaries" << CFendl;
    for (Uint dict_idx=0; dict_idx<m_mesh->dictionaries().size(); ++dict_idx)
    {
      Dictionary& dict = *m_mesh->dictionaries()[dict_idx];
      dict.rebuild_node_to_element_connectivity();
    }
    node_elem_connectivity_needs_rebuild = false;
  }
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::move_elements(const std::vector< std::vector< std::vector<Uint> > >& exported_elements_loc_id)
{
  cf3_assert(exported_elements_loc_id.size() == PE::Comm::instance().size());

  // Procedure:
  // 0) mark elements for removal
  // 1) send elements in meantime building a set of nodes_to_send
  // 2) add/remove elements, flush elements
  // 3) send nodes
  // 4) add nodes
  // 5) flush nodes
  // 6) remove unused nodes
  // 7) flush nodes, rebuild glb_to_local node map
  // 8) fix rank of nodes: lowest rank that is found is assigned

  const Uint nb_dicts = m_mesh->dictionaries().size();
  const Uint nb_entities = m_mesh->elements().size();

  // 1) - Change rank of elements to where they need to be moved,
  //    - Mark elements for removal
  for (Uint pid=0; pid<PE::Comm::instance().size(); ++pid)
  {
    cf3_assert(exported_elements_loc_id[pid].size() == m_mesh->elements().size());
    for (Uint entities_idx=0; entities_idx<m_mesh->elements().size(); ++entities_idx)
    {
      Entities& entities = *m_mesh->elements()[entities_idx];

      boost_foreach (const Uint loc_elem_idx, exported_elements_loc_id[pid][entities_idx])
      {
        // Change rank to where it needs to be moved
        entities.rank()[loc_elem_idx] = pid;

        // Mark element for removal
        remove_element(entities_idx,loc_elem_idx);
      }
    }
  }

  //PECheckArrivePoint(100,"Sending elements");

  // 2) send elements and nodes belonging to the elements
  std::vector< std::vector< std::vector<boost::uint64_t> > > imported_elements_glb_id;
  send_elements(exported_elements_loc_id,imported_elements_glb_id);


  //PECheckArrivePoint(100,"Finding nodes to export");
  std::vector< std::vector< std::vector<Uint> > > exported_nodes_loc_id;
  find_nodes_to_export(exported_elements_loc_id,exported_nodes_loc_id);

//  if (nb_dicts==2)
//  {
//    PEProcessSortedExecute(-1,
//    for (Uint pid=0; pid<PE::Comm::instance().size(); ++pid)
//    {
//      if (pid != PE::Comm::instance().rank())
//      {
//        for (Uint entities_idx=0; entities_idx<nb_entities; ++entities_idx)
//        {
//          std::vector<Uint> exported_glb_elems; exported_glb_elems.reserve(exported_elements_loc_id[pid][entities_idx].size());
//          boost_foreach (const Uint elem, exported_elements_loc_id[pid][entities_idx] )
//          {
//            exported_glb_elems.push_back( m_mesh->elements()[entities_idx]->glb_idx()[elem]+1 );
//          }
//          std::cout << PERank << m_mesh->elements()[entities_idx]->uri() << "  :  " << to_str(exported_glb_elems) << std::endl;
//        }
//      }
//    }

//    for (Uint pid=0; pid<PE::Comm::instance().size(); ++pid)
//    {
//      if (pid != PE::Comm::instance().rank())
//      {
//        std::vector<Real> exported_glb_nodes; exported_glb_nodes.reserve(exported_nodes_loc_id[pid][1].size());
//        boost_foreach (const Uint node, exported_nodes_loc_id[pid][1] )
//        {
//          exported_glb_nodes.push_back( m_mesh->dictionaries()[1]->field("glb_elem")[node][0] );
//        }
//        std::cout << PERank << to_str(exported_glb_nodes) << std::endl;
//      }
//    }
//    )
//  }



  //PECheckArrivePoint(100,"Sending nodes");

  std::vector< std::vector< std::vector<boost::uint64_t> > > imported_nodes_glb_id;
  send_nodes(exported_nodes_loc_id,imported_nodes_glb_id);

  flush_elements();

  // 6) Remove unused nodes
  //PECheckArrivePoint(100,"removing unused nodes");

  for (Uint dict_idx=0; dict_idx<nb_dicts; ++dict_idx)
  {
    //std::cout << PERank << "removing unused nodes " << std::endl;

    Dictionary& dict = *m_mesh->dictionaries()[dict_idx];

    // Assemble set of used nodes, that will be checked for later
    std::set<boost::uint64_t> used_nodes;

    // check in dict.entities_range(), in case perhaps other meshes use the same dictionary (future?)
    cf3_assert(dict.entities_range().size() != 0);
    boost_foreach (const Handle<Entities>& entities, dict.entities_range())
    {
      //std::cout << entities->uri() << std::endl;
      Space& space = entities->space(dict);
      for (Uint elem=0; elem<space.size(); ++elem)
      {
        // Element-node connectivity tables must be GLOBAL
        boost_foreach( Uint glb_node, space.connectivity()[elem] )
        {
          used_nodes.insert(glb_node);
        }
      }
    }

    // Remove unused nodes
    for (Uint node_idx=0; node_idx<dict.size(); ++node_idx)
    {
      if ( used_nodes.count(dict.glb_idx()[node_idx]) == 0 )
      {
        remove_node(dict_idx,node_idx);
      }
    }
  }

  // 7) Flush nodes and rebuild glb_to_loc map
//  CFdebug << "Flush nodes and rebuild glb_to_loc map" << CFendl;
  flush_nodes();
//  CFdebug << "Flushed nodes" << CFendl;

  // 8) Fix node ranks

  // call function to fix the ranks
  fix_node_ranks();


//  cf3_assert(added_elements.empty());
//  cf3_assert(added_nodes.empty());
//  finish();
//  if (nb_dicts==2)
//  {
//    typedef std::map<boost::uint64_t,Uint> Map_t;
//    PEProcessSortedExecute(-1,
//    for (Uint pid=0; pid<PE::Comm::instance().size(); ++pid)
//    {
//      if (pid != PE::Comm::instance().rank())
//      {
//        for (Uint entities_idx=0; entities_idx<nb_entities; ++entities_idx)
//        {
//          Map_t glb_to_loc;
//          for (Uint e=0; e<m_mesh->elements()[entities_idx]->size(); ++e)
//          {
//            glb_to_loc[ m_mesh->elements()[entities_idx]->glb_idx()[e] ] = e;
//          }

//          const Space& space = m_mesh->elements()[entities_idx]->space(*m_mesh->dictionaries()[1]);
//          std::vector<Uint> imported_glb_elems; imported_glb_elems.reserve(imported_elements_glb_id[pid][entities_idx].size());
//          boost_foreach (const Uint glb_elem, imported_elements_glb_id[pid][entities_idx] )
//          {
//            imported_glb_elems.push_back( glb_elem+1 );
//            Uint loc_node;
//            if (is_node_connectivity_global)
//              loc_node = m_mesh->dictionaries()[1]->glb_to_loc()[ space.connectivity()[glb_to_loc[glb_elem]][0] ];
//            else
//              loc_node = space.connectivity()[glb_to_loc[glb_elem]][0];
//            Real field_val = m_mesh->dictionaries()[1]->field("glb_elem")[loc_node][0];
//            std::cout << PERank << "glb_elem " << glb_elem+1 << "  value " << m_mesh->dictionaries()[1]->field("glb_elem")[loc_node][0] << std::endl;
//            cf3_assert_desc(to_str(field_val)+"=="+to_str(glb_elem+1),field_val == glb_elem+1);
//          }
//          std::cout << PERank << m_mesh->elements()[entities_idx]->uri() << "  :  " << to_str(imported_glb_elems) << std::endl;

//        }
//      }
//    }

//    for (Uint pid=0; pid<PE::Comm::instance().size(); ++pid)
//    {
//      if (pid != PE::Comm::instance().rank())
//      {
//        std::vector<Real> imported_glb_nodes; imported_glb_nodes.reserve(imported_nodes_glb_id[pid][1].size());
//        boost_foreach (const Uint glb_node, imported_nodes_glb_id[pid][1] )
//        {
//          imported_glb_nodes.push_back( m_mesh->dictionaries()[1]->field("glb_elem")[m_mesh->dictionaries()[1]->glb_to_loc()[glb_node]][0] );
//        }
//        std::cout << PERank << to_str(imported_glb_nodes) << std::endl;
//      }
//    }
//    )
//  }


}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::fix_node_ranks()
{
  CFdebug << "MeshAdaptor: fix node ranks" << CFendl;
  //PECheckArrivePoint(100,"fix node ranks");
  const Uint nb_dicts = m_mesh->dictionaries().size();

  flush_nodes();
  rebuild_node_glb_to_loc_map();
  boost_foreach (const Handle<Dictionary>& dict, m_mesh->dictionaries())
  {
    // nodes to send to other cpu's
    std::vector<boost::uint64_t> glb_nodes(dict->size());
    for (Uint n=0; n<glb_nodes.size(); ++n)
    {
      // fill the nodes to send to other cpu's
      glb_nodes[n] = dict->glb_idx()[n];

      // assign the rank to be ourself to start with. The rank can only become lower.
      cf3_assert(n<dict->rank().size());
      dict->rank()[n] = PE::Comm::instance().rank();
    }

    // Every cpu broadcasts to cpu's with higher rank
    for (Uint root=0; root<PE::Comm::instance().size(); ++root)
    {
      //    CFinfo << "broadcasting from root " << root << CFendl;
      PE::Comm::instance().barrier();

      std::vector<boost::uint64_t> glb_nodes_from_root(0);
      PE::Comm::instance().broadcast(glb_nodes,glb_nodes_from_root,root);

      // cpu's with higher rank will adapt the ranks of the nodes
      if (PE::Comm::instance().rank() > root)
      {

        boost_foreach(const boost::uint64_t& glb_node, glb_nodes_from_root)
        {
          if (dict->glb_to_loc().exists(glb_node))
          {
            Uint loc_node = dict->glb_to_loc()[glb_node];
            cf3_assert(loc_node<dict->rank().size());
            // if the rank is yourself, it means that the glb_node has not been found
            // before. Change then the rank to the broacasting cpu.
            if (dict->rank()[loc_node] == PE::Comm::instance().rank())
            {
              dict->rank()[loc_node] = root;
            }
          }
        }
      }
    }
  }

//  //PECheckArrivePoint(100,"fix node ranks done");

}

////////////////////////////////////////////////////////////////////////////////

#if 0
void MeshAdaptor::fix_node_ranks(const std::vector< std::vector<boost::uint64_t> >& request_node_ranks)
{
 Following is flawed if the request_node_ranks are not completely covering everything
  // For every dictionary
  for (Uint dict_idx=0; dict_idx<nb_dicts; ++dict_idx)
  {
    cf3_assert(m_mesh->dictionaries()[dict_idx]);
    Dictionary& dict = *m_mesh->dictionaries()[dict_idx];

    // Gather from all processors nodes to see if we have them.
    std::vector< std::vector<boost::uint64_t> > recv_request_node_ranks(Comm::instance().size());
    if (Comm::instance().is_active())
      Comm::instance().all_gather(request_node_ranks[dict_idx],recv_request_node_ranks);
    else
      recv_request_node_ranks[0] = request_node_ranks[dict_idx];

    //PECheckArrivePoint(100,"communicated 1");

    // Search if this rank contains the missing node-ranks given in recv_request_node_ranks,
    // and mark the node as found.
    std::vector< std::vector<int> > send_found_on_rank(Comm::instance().size());
    for (Uint pid=0; pid<Comm::instance().size(); ++pid)
    {
      send_found_on_rank[pid].resize(recv_request_node_ranks[pid].size(),false);
      {
        for (Uint node_idx=0; node_idx<recv_request_node_ranks[pid].size(); ++node_idx)
        {
          if ( dict.glb_to_loc().exists(recv_request_node_ranks[pid][node_idx]) )
          {
            send_found_on_rank[pid][node_idx] = true;
            const Uint loc_idx = dict.glb_to_loc()[recv_request_node_ranks[pid][node_idx]];
            dict.rank()[loc_idx] = std::min(dict.rank()[loc_idx],pid);
          }
        }
      }
    }

    // Communicate which processes found the missing ranks
    std::vector< std::vector<int> > recv_found_requested_nodes(Comm::instance().size());
    if (Comm::instance().is_active())
      Comm::instance().all_to_all(send_found_on_rank,recv_found_requested_nodes);
    else
      recv_found_requested_nodes[0] = send_found_on_rank[0];

    //PECheckArrivePoint(100,"communicated 2");

    rebuild_node_glb_to_loc_map();

    //PECheckArrivePoint(100,"rebuilt map");

    // Set the missing rank to the lowest pid that found it
    for (Uint node_idx=0; node_idx<request_node_ranks[dict_idx].size(); ++node_idx)
    {
      Uint rank_that_owns = math::Consts::uint_max();

      for (Uint pid=0; pid<Comm::instance().size(); ++pid)
      {
        if (recv_found_requested_nodes[pid][node_idx])
        {
          cf3_assert( dict.glb_to_loc().exists(request_node_ranks[dict_idx][node_idx]) );
          cf3_assert( dict.glb_to_loc()[request_node_ranks[dict_idx][node_idx]] < dict.rank().size() );
          dict.rank()[ dict.glb_to_loc()[request_node_ranks[dict_idx][node_idx]] ] = pid;
          break;
        }
      }
      cf3_assert(dict.rank()[ dict.glb_to_loc()[request_node_ranks[dict_idx][node_idx]] ] < PE::Comm::instance().size());
    }
  }
}
#endif

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::grow_overlap()
{
  flush_nodes();
  flush_elements();

  const Uint nb_dicts = m_mesh->dictionaries().size();

  CFdebug << "MeshAdaptor: finding bdry nodes" << CFendl;

  // It is assumed element-node connectivity is LOCAL to find the face2cell connectivity
  restore_element_node_connectivity();

  boost::shared_ptr<FaceCellConnectivity> face2cell = allocate_component<FaceCellConnectivity>("face2cell");
  face2cell->setup(m_mesh->topology());

  Dictionary& geometry_dict = m_mesh->geometry_fields();

  std::set<boost::uint64_t> bdry_nodes;
  for (Uint f=0; f<face2cell->size(); ++f)
  {
    cf3_assert(f < face2cell->is_bdry_face().size());
    if (face2cell->is_bdry_face()[f])
    {
      boost_foreach(const Uint node, face2cell->face_nodes(f))
      {
        cf3_assert(node<geometry_dict.glb_idx().size());
        bdry_nodes.insert(geometry_dict.glb_idx()[node]);
      }
    }
  }
  // Remove from memory again, boundary nodes are found
  face2cell.reset();

  ////PECheckArrivePoint(100, "boundary nodes found");

  // Copy set into vector
  std::vector<boost::uint64_t> glb_boundary_nodes;
  glb_boundary_nodes.reserve(bdry_nodes.size());
  boost_foreach (Uint node, bdry_nodes)
  {
    glb_boundary_nodes.push_back(node);
  }
  bdry_nodes.clear();

  rebuild_node_to_element_connectivity();
  //std::cout << PERank << geometry_dict.connectivity() << std::endl;


  // We now have a vector of nodes that lie at the boundary of each pid's mesh
  // Now find on other pid's the elements that share these nodes, and create
  // a elements_changeset.

  std::vector< std::vector<boost::uint64_t> > recv_elem_glb_nodes;
  PE::Comm::instance().all_gather(glb_boundary_nodes, recv_elem_glb_nodes);
  cf3_assert(recv_elem_glb_nodes.size() == PE::Comm::instance().size());

  ////PECheckArrivePoint(100, "boundary nodes allgathered");

  std::vector< std::vector< std::vector< Uint > > > exported_elements_loc_id (PE::Comm::instance().size(),
                                                                              std::vector< std::vector<Uint> > (m_mesh->elements().size()));

  PE::Buffer send_buffer, receive_buffer;
  cf3_assert(geometry_dict.connectivity().size() == geometry_dict.size());

  rebuild_node_glb_to_loc_map();

  for (Uint pid=0; pid<PE::Comm::instance().size(); ++pid)
  {
    if (pid != PE::Comm::instance().rank())
    {
      boost_foreach (const boost::uint64_t& glb_node, recv_elem_glb_nodes[pid])
      {
        if (geometry_dict.glb_to_loc().exists(glb_node))
        {
          const Uint loc_node = geometry_dict.glb_to_loc()[glb_node];
          cf3_assert(loc_node<geometry_dict.size());
//          CFdebug << "[0] found glb_node " << glb_node+1 <<      "   :";
          boost_foreach(const SpaceElem& elem, geometry_dict.connectivity()[loc_node])
          {
//            CFdebug << " " << elem.glb_idx()+1;
            //if (elem.rank() == PE::Comm::instance().rank())
            {
              const Uint entities_idx = elem.comp->support().entities_idx();
              exported_elements_loc_id[pid][entities_idx].push_back(elem.idx);
            }
          }
//          CFdebug << CFendl;
        }
      }
    }
  }

  ////PECheckArrivePoint(100, "exported_elements_loc_id assembled");




  std::vector< std::vector< std::vector<Uint> > >            exported_nodes_loc_id;
  find_nodes_to_export(exported_elements_loc_id,exported_nodes_loc_id);

  std::vector< std::vector< std::vector<boost::uint64_t> > > imported_elems_glb_id;
  send_elements(exported_elements_loc_id,imported_elems_glb_id);

  flush_elements();

  std::vector< std::vector< std::vector<boost::uint64_t> > > imported_nodes_glb_id;
  send_nodes(exported_nodes_loc_id,imported_nodes_glb_id);

  flush_nodes();
  // nodes and elements should be flushed now, as well as dict.glb_to_loc rebuilt.
  // A call to finish() should restore the element-node connectivity tables and update statistics
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
