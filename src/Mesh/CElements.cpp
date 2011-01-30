// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "Common/Log.hpp"
#include "Common/CLink.hpp"
#include "Common/CGroup.hpp"
#include "Common/CreateComponent.hpp"

#include "Mesh/CElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CSpace.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CElements, CEntities, LibMesh > CElements_Builder;

////////////////////////////////////////////////////////////////////////////////

CElements::CElements ( const std::string& name ) :
  CEntities ( name )
{
  properties()["brief"] = std::string("Holds information of elements of one type");
  properties()["description"] = std::string("Container component that stores the element to node connectivity,\n")
  +std::string("a link to node storage, a list of used nodes, and global numbering unique over all processors");
  
  m_connectivity_table = create_static_component<CTable<Uint> >("connectivity_table");
  m_connectivity_table->add_tag("connectivity_table");
  m_connectivity_table->properties()["brief"] = std::string("The connectivity table specifying for each element the nodes in the coordinates table");
  
  
  
  /// @todo remove
  properties()["element_based"] = false;
  properties()["node_based"] = false;
  
}

////////////////////////////////////////////////////////////////////////////////

CElements::~CElements()
{
}

//////////////////////////////////////////////////////////////////////////////

void CElements::initialize(const std::string& element_type_name, CNodes& nodes)
{
  CEntities::initialize(element_type_name,nodes);
  connectivity_table().set_row_size(m_element_type->nb_nodes());
}

//////////////////////////////////////////////////////////////////////////////

CTable<Uint>& CElements::connectivity_table()
{
  return *m_connectivity_table;
}

//////////////////////////////////////////////////////////////////////////////

const CTable<Uint>& CElements::connectivity_table() const
{
  return *m_connectivity_table;
}

//////////////////////////////////////////////////////////////////////////////

void CElements::add_field_elements_link(CElements& field_elements)
{
  CGroup::Ptr field_group = get_child<CGroup>("fields");
  if ( is_null(field_group) )
    field_group = create_component<CGroup>("fields");

  const std::string field_name = field_elements.get_parent()->as_type<CField>()->field_name();
  field_group->create_component<CLink>(field_name)->link_to(field_elements.follow());
}

//////////////////////////////////////////////////////////////////////////////

CElements& CElements::get_field_elements(const std::string& field_name)
{
  Component::Ptr all_fields = get_child("fields");
  cf_assert(all_fields.get());
  Component::Ptr field = all_fields->get_child(field_name);
  cf_assert(field.get());
  return *field->as_type<CElements>();
}
  
//////////////////////////////////////////////////////////////////////////////

const CElements& CElements::get_field_elements(const std::string& field_name) const
{
  Component::ConstPtr all_fields = get_child("fields");
  cf_assert(all_fields.get());
  Component::ConstPtr field = all_fields->get_child(field_name);
  cf_assert(field.get());
  return *field->as_type<CElements const>();
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix CElements::get_coordinates(const Uint elem_idx) const
{
  const CTable<Real>& coords_table = nodes().coordinates();
  CTable<Uint>::ConstRow elem_nodes = connectivity_table()[elem_idx];

  const Uint nb_nodes=elem_nodes.size();
  const Uint dim=coords_table.row_size();
  RealMatrix elem_coords(nb_nodes,dim);
  
  for(Uint node = 0; node != nb_nodes; ++node)
    for (Uint d=0; d<dim; ++d)
      elem_coords(node,d) = coords_table[elem_nodes[node]][d];

  return elem_coords;
}

////////////////////////////////////////////////////////////////////////////////

void CElements::put_coordinates(RealMatrix& elem_coords, const Uint elem_idx) const
{
  const CTable<Real>& coords_table = nodes().coordinates();
  CTable<Uint>::ConstRow elem_nodes = connectivity_table()[elem_idx];

  const Uint nb_nodes=elem_coords.rows();
  const Uint dim=elem_coords.cols();
 
  
  for(Uint node = 0; node != nb_nodes; ++node)
    for (Uint d=0; d<dim; ++d)
      elem_coords(node,d) = coords_table[elem_nodes[node]][d];
}

////////////////////////////////////////////////////////////////////////////////

CTable<Uint>::ConstRow CElements::get_nodes(const Uint elem_idx)
{
  return connectivity_table()[elem_idx];
//  CTable<Uint>::ConstRow elem_nodes = connectivity_table(space)[elem_idx];
//  return std::vector<Uint> (elem_nodes.begin(),elem_nodes.end());
}

////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CElements::initialize(CElements& elements)
{
  add_tag("field_elements");
  CEntities::initialize(elements.element_type().builder_name(),elements.nodes());

  m_support = create_static_component<CLink>("support");
  m_support->link_to(elements.follow());
  m_support->add_tag("support");
  
  m_connectivity_table = elements.connectivity_table().as_type<CTable<Uint> >();
}

////////////////////////////////////////////////////////////////////////////////

void CElements::add_node_based_storage(CTable<Real>& nodal_data)
{
  // Set the nodal data
  m_data_name = "node_data";
  CLink::Ptr node_data = create_component<CLink>(m_data_name);
  nodal_data.add_tag(m_data_name);
	node_data->add_tag(m_data_name);
  node_data->link_to(nodal_data.follow());
  properties()["node_based"] = true;
}

////////////////////////////////////////////////////////////////////////////////

void CElements::add_element_based_storage()
{
  // Create elemental data
  m_data_name = "element_data";
  CTable<Real>::Ptr elm_data = create_component<CTable<Real> >(m_data_name);
  elm_data->add_tag(m_data_name);
	elm_data->add_tag("field_data");
  properties()["element_based"] = true;
}

//////////////////////////////////////////////////////////////////////////////

CTable<Real>& CElements::data()
{
  Component& data = find_component_with_filter(*this,IsComponentTag(m_data_name));
  return *data.follow()->as_type<CTable<Real> >();
}

//////////////////////////////////////////////////////////////////////////////

const CTable<Real>& CElements::data() const
{
  const Component& data = find_component_with_filter(*this,IsComponentTag(m_data_name));
  return *data.follow()->as_type<CTable<Real> const>();
}

//////////////////////////////////////////////////////////////////////////////

CElements& CElements::get_geometry_elements()
{
  return *m_support->follow()->as_type<CElements>();
}

//////////////////////////////////////////////////////////////////////////////

const CElements& CElements::get_geometry_elements() const
{
  return *m_support->follow()->as_type<CElements>();
}

////////////////////////////////////////////////////////////////////////////////

CSpace& CElements::create_space0()
{
  cf_assert(m_spaces.size() == 0);
  CSpace::Ptr space = create_component<CSpace>("space[0]");
  space->initialize(*this);
  m_spaces.push_back(space);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
