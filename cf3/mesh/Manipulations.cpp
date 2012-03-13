// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/FindComponents.hpp"
#include "common/Map.hpp"
#include "common/PropertyList.hpp"

#include "common/PE/debug.hpp"

#include "math/Consts.hpp"

#include "mesh/Manipulations.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Space.hpp"
#include "mesh/Mesh.hpp"

namespace cf3 {
namespace mesh {

using namespace common;
using namespace common::PE;
using namespace math::Consts;

////////////////////////////////////////////////////////////////////////////////

PackedElement::PackedElement(const mesh::Mesh& mesh, common::PE::Buffer& buffer) : m_mesh(mesh)
{
  unpack(buffer);
}

////////////////////////////////////////////////////////////////////////////////

PackedElement::PackedElement(const mesh::Mesh& mesh, const Uint entities_idx, const Uint elem_loc_idx) : m_mesh(mesh)
{
  Uint nb_nodes;
  m_entities_idx = entities_idx;
  m_loc_idx = elem_loc_idx;
  cf3_assert(m_entities_idx < mesh.elements().size());
  const Handle<Entities>& entities = mesh.elements()[entities_idx];
  cf3_assert(elem_loc_idx < entities->size());
  m_glb_idx = entities->glb_idx()[m_loc_idx];
  m_rank = entities->rank()[m_loc_idx];
  m_glb_connectivity.resize(entities->spaces().size());
  for (Uint space_idx=0; space_idx<m_glb_connectivity.size(); ++space_idx)
  {
    const Dictionary& dict = entities->spaces()[space_idx]->dict();
    nb_nodes = entities->spaces()[space_idx]->shape_function().nb_nodes();
    m_glb_connectivity[space_idx].resize(nb_nodes);
    for (Uint node=0; node<nb_nodes; ++node)
    {
      cf3_assert(m_loc_idx < entities->spaces()[space_idx]->connectivity().size());
      cf3_assert(entities->spaces()[space_idx]->connectivity()[m_loc_idx][node] < dict.size() );
      m_glb_connectivity[space_idx][node] = dict.glb_idx()[ entities->spaces()[space_idx]->connectivity()[m_loc_idx][node] ];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void PackedElement::unpack(PE::Buffer& buf)
{
  Uint nb_spaces;
  buf >> m_entities_idx >> m_loc_idx >> m_glb_idx >> m_rank >> nb_spaces;
  m_glb_connectivity.resize(nb_spaces);
  for (Uint space_idx=0; space_idx<nb_spaces; ++space_idx)
  {
    buf >> m_glb_connectivity[space_idx];
  }
  std::cout << PERank << "unpacked element    glb_idx = " << m_glb_idx << "\t    rank = " << m_rank << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void PackedElement::pack(PE::Buffer& buf)
{
  buf << m_entities_idx << m_loc_idx << m_glb_idx << m_rank << (Uint) m_glb_connectivity.size();
  for (Uint space_idx=0; space_idx<m_glb_connectivity.size(); ++space_idx)
  {
    buf << m_glb_connectivity[space_idx];
  }
  std::cout << PERank << "packed element    glb_idx = " << m_glb_idx << "\t    rank = " << m_rank << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

PackedNode::PackedNode(const mesh::Mesh& mesh, common::PE::Buffer& buffer) : m_mesh(mesh)
{
  unpack(buffer);
}

////////////////////////////////////////////////////////////////////////////////

PackedNode::PackedNode(const mesh::Mesh& mesh, const Uint dict_idx, const Uint node_loc_idx): m_mesh(mesh)
{
  m_dict_idx = dict_idx;
  cf3_assert(m_dict_idx<mesh.dictionaries().size());
  const Dictionary& dict = *mesh.dictionaries()[m_dict_idx];
  m_loc_idx = node_loc_idx;
  m_glb_idx = dict.glb_idx()[m_loc_idx];
  m_rank = dict.rank()[m_loc_idx];
  m_field_values.resize(dict.fields().size());
  for (Uint fields_idx=0; fields_idx<m_field_values.size(); ++fields_idx)
  {
    Field& field = *dict.fields()[fields_idx];
    m_field_values[fields_idx].resize(field.row_size());
    for (Uint var=0; var<field.row_size(); ++var)
    {
      m_field_values[fields_idx][var] = field[m_loc_idx][var];
    }
  }
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
  std::cout << PERank << "unpacked node    glb_idx = " << m_glb_idx << "\t    rank = " << m_rank << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void PackedNode::pack(PE::Buffer& buf)
{
  buf << m_dict_idx << m_loc_idx << m_glb_idx << m_rank << (Uint) m_field_values.size();
  for (Uint fields_idx=0; fields_idx<m_field_values.size(); ++fields_idx)
  {
    buf << m_field_values[fields_idx];
  }
  std::cout << PERank << "packed node    glb_idx = " << m_glb_idx << "\t    rank = " << m_rank << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

MeshAdaptor::MeshAdaptor(mesh::Mesh &mesh)
{
  is_node_connectivity_global = false;
  m_mesh = mesh.handle<Mesh>();
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::prepare()
{
  make_element_node_connectivity_global();
  create_element_buffers();
  create_node_buffers();
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::finish()
{
  flush_nodes();
  flush_elements();
  restore_element_node_connectivity();
  cf3_assert( ! is_node_connectivity_global );

  /// @todo dictionary global_2_local map must be remade!

  // Update the mesh
  m_mesh->update_statistics();
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
    boost_foreach (Entities& elements, find_components_recursively<Entities>(*m_mesh))
    {
      boost_foreach(const Handle<Space>& space, elements.spaces())
      {
        common::Map<boost::uint64_t,Uint>& glb_to_loc = space->dict().glb_to_loc();
        boost_foreach ( Connectivity::Row nodes, space->connectivity().array() )
        {
          boost_foreach ( Uint& node, nodes )
          {
            node = glb_to_loc[node];
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
  cf3_assert_desc("Forgot to call MeshAdaptor::create_element_buffers()",element_glb_idx.size());
  cf3_assert(is_node_connectivity_global);
  element_glb_idx[packed_element.entities_idx()]->add_row(packed_element.glb_idx());
  element_rank[packed_element.entities_idx()]->add_row(packed_element.rank());
  for (Uint space_idx=0; space_idx<element_connected_nodes[packed_element.entities_idx()].size(); ++space_idx)
  {
    cf3_assert(packed_element.glb_connectivity()[space_idx].size() == element_connected_nodes[packed_element.entities_idx()][space_idx]->get_appointed().shape()[1]);
    element_connected_nodes[packed_element.entities_idx()][space_idx]->add_row(packed_element.glb_connectivity()[space_idx]);
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
  cf3_assert_desc("Forgot to call MeshAdaptor::create_element_buffers()",element_glb_idx.size());
  cf3_assert(entities_idx< element_glb_idx.size() );
  cf3_assert(elem_loc_idx < element_glb_idx[entities_idx]->total_allocated());
  element_glb_idx[entities_idx]->rm_row(elem_loc_idx);
  element_rank[entities_idx]->rm_row(elem_loc_idx);
  for (Uint space_idx=0; space_idx<element_connected_nodes[entities_idx].size(); ++space_idx)
    element_connected_nodes[entities_idx][space_idx]->rm_row(elem_loc_idx);
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::add_node(const PackedNode& packed_node)
{
  cf3_assert_desc("Forgot to call MeshAdaptor::create_element_buffers()",node_glb_idx.size());
  node_glb_idx[packed_node.dict_idx()]->add_row(packed_node.glb_idx());
  node_rank[packed_node.dict_idx()]->add_row(packed_node.rank());
  for (Uint fields_idx=0; fields_idx<node_field_values[packed_node.dict_idx()].size(); ++fields_idx)
  {
    cf3_assert(packed_node.field_values()[fields_idx].size() == node_field_values[packed_node.dict_idx()][fields_idx]->get_appointed().shape()[1]);
    node_field_values[packed_node.dict_idx()][fields_idx]->add_row(packed_node.field_values()[fields_idx]);
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
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::flush_elements()
{
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
}

////////////////////////////////////////////////////////////////////////////////

void MeshAdaptor::flush_nodes()
{
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
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

RemoveNodes::RemoveNodes(Dictionary& nodes) :
    glb_idx (nodes.glb_idx().create_buffer()),
    rank (nodes.rank().create_buffer()),
    coordinates (nodes.coordinates().create_buffer()),
    connected_elements (nodes.glb_elem_connectivity().create_buffer())
{}

////////////////////////////////////////////////////////////////////////////////

void RemoveNodes::operator() (const Uint idx)
{
  Uint val = glb_idx.get_row(idx);

  glb_idx.rm_row(idx);
  rank.rm_row(idx);
  coordinates.rm_row(idx);
  connected_elements.rm_row(idx);

//  std::cout << PERank << "removed node  " << val << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void RemoveNodes::flush()
{
  glb_idx.flush();
  rank.flush();
  coordinates.flush();
  connected_elements.flush();
}

////////////////////////////////////////////////////////////////////////////////

RemoveElements::RemoveElements(Entities& elements) :
    glb_idx (elements.glb_idx().create_buffer()),
    rank (elements.rank().create_buffer()),
    connected_nodes (elements.geometry_space().connectivity().create_buffer())
{}

////////////////////////////////////////////////////////////////////////////////

void RemoveElements::operator() (const Uint idx)
{
  Uint val = glb_idx.get_row(idx);

  glb_idx.rm_row(idx);
  rank.rm_row(idx);
  connected_nodes.rm_row(idx);

//  std::cout << PERank << "removed element  " << val << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void RemoveElements::flush()
{
  glb_idx.flush();
  rank.flush();
  connected_nodes.flush();
}

////////////////////////////////////////////////////////////////////////////////

PackUnpackElements::PackUnpackElements(Entities& elements) :
    m_elements(elements),
    m_remove_after_pack(false),
    m_idx(uint_max()),
    glb_idx (elements.glb_idx().create_buffer()),
    rank (elements.rank().create_buffer()),
    connected_nodes (elements.geometry_space().connectivity().create_buffer())
{}

////////////////////////////////////////////////////////////////////////////////

PackUnpackElements& PackUnpackElements::operator() (const Uint idx, const bool remove_after_pack)
{
  m_idx=idx;
  m_remove_after_pack = remove_after_pack;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackElements::remove(const Uint idx)
{
  Uint val = glb_idx.get_row(idx);

  glb_idx.rm_row(idx);
  rank.rm_row(idx);
  connected_nodes.rm_row(idx);

  //std::cout << PERank << "removed element  " << val << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackElements::pack(PE::Buffer& buf)
{
  cf3_assert_desc("Must call using  object(idx).pack(buf), instead of object.pack(buf)" , m_idx != uint_max());

  Uint val = m_elements.glb_idx()[m_idx];

  buf << m_elements.glb_idx()[m_idx]
      << m_elements.rank()[m_idx];

  boost_foreach(const Uint connected_node, m_elements.geometry_space().connectivity()[m_idx])
      buf << connected_node;

  //std::cout << PERank << "packed element    glb_idx = " << val << std::endl;

  if (m_remove_after_pack)
    remove(m_idx);

}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackElements::unpack(PE::Buffer& buf)
{
  Uint glb_idx_data;
  Uint rank_data;
  std::vector<Uint> connected_nodes_data(m_elements.geometry_space().connectivity().row_size());

  buf >> glb_idx_data >> rank_data;

  for (Uint n=0; n<connected_nodes_data.size(); ++n)
    buf >> connected_nodes_data[n];

  Uint idx;
  idx = glb_idx.add_row(glb_idx_data);
  cf3_always_assert(rank.add_row(rank_data) == idx);
  cf3_always_assert(connected_nodes.add_row(connected_nodes_data) == idx);

  // std::cout << PERank << "unpacked and added element    glb_idx = " << glb_idx_data << "\t    rank = " << rank_data << "\t    connected_nodes = " << connected_nodes_data << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackElements::flush()
{
  glb_idx.flush();
  connected_nodes.flush();
  rank.flush();
}

////////////////////////////////////////////////////////////////////////////////

PackUnpackNodes::PackUnpackNodes(Dictionary& nodes) :
  m_nodes(nodes),
  m_remove_after_pack(false),
  m_idx(uint_max()),
  glb_idx (nodes.glb_idx().create_buffer(100)),
  rank (nodes.rank().create_buffer(100)),
  coordinates (nodes.coordinates().create_buffer(100)),
  connected_elements (nodes.glb_elem_connectivity().create_buffer(100))
{}

////////////////////////////////////////////////////////////////////////////////

PackUnpackNodes& PackUnpackNodes::operator() (const Uint idx, const bool remove_after_pack)
{
  m_idx=idx;
  m_remove_after_pack = remove_after_pack;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackNodes::remove(const Uint idx)
{
  Uint val = glb_idx.get_row(idx);

  cf3_assert(idx < m_nodes.size());

  glb_idx.rm_row(idx);
  rank.rm_row(idx);
  coordinates.rm_row(idx);
  connected_elements.rm_row(idx);

  m_idx = uint_max();
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackNodes::pack(PE::Buffer& buf)
{
  cf3_assert_desc("Must call using  object(idx).pack(buf), instead of object.pack(buf)" , m_idx != uint_max());

  cf3_assert_desc("["+to_str(m_idx)+">="+to_str(m_nodes.size())+"]",m_idx < m_nodes.size());
  cf3_assert(m_idx < m_nodes.glb_idx().size());
  cf3_assert(m_idx < m_nodes.rank().size());
  cf3_assert(m_idx < m_nodes.coordinates().size());
  cf3_assert(m_idx < m_nodes.glb_elem_connectivity().size());


  Uint val = m_nodes.glb_idx()[m_idx];

  buf << m_nodes.glb_idx()[m_idx];

  buf << m_nodes.rank()[m_idx];

  buf << m_nodes.coordinates()[m_idx];

  buf << m_nodes.glb_elem_connectivity()[m_idx];

//  std::cout << PERank << "packed node    glb_idx = " << val << std::endl;

  if (m_remove_after_pack)
    remove(m_idx);

  m_idx = uint_max();
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackNodes::unpack(PE::Buffer& buf)
{
  Uint glb_idx_data;
  Uint rank_data;
  std::vector<Real> coordinates_data;
  std::vector<Uint> connected_elems_data;

  buf >> glb_idx_data >> rank_data >> coordinates_data >> connected_elems_data;

  Uint idx;
             idx = glb_idx.add_row(glb_idx_data);
  cf3_always_assert(rank.add_row(rank_data) == idx);
  cf3_always_assert(coordinates.add_row(coordinates_data) == idx);
  cf3_always_assert(connected_elements.add_row(connected_elems_data) == idx);

//  std::cout << PERank << "added node    glb_idx = " << glb_idx_data << "\t    rank = " << rank_data << "\t    coords = " << to_str(coordinates_data) << "\t    connected_elem = " << to_str(connected_elems_data) << std::endl;
  m_idx = uint_max();
}

////////////////////////////////////////////////////////////////////////////////

void PackUnpackNodes::flush()
{
  glb_idx.flush();
  rank.flush();
  coordinates.flush();
  connected_elements.flush();
  m_nodes.resize(m_nodes.coordinates().size());
  m_idx = uint_max();
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
