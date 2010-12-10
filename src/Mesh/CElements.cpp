// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "Common/CLink.hpp"
#include "Common/CGroup.hpp"
#include "Common/CreateComponent.hpp"

#include "Mesh/CElements.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CList.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ComponentBuilder < CElements, Component, LibMesh > CElements_Builder;

////////////////////////////////////////////////////////////////////////////////

CElements::CElements ( const std::string& name ) :
  Component ( name )
{
  m_connectivity_table = create_static_component<CTable<Uint> >("connectivity_table");
  m_connectivity_table->add_tag("connectivity_table");
  m_connectivity_table->properties()["brief"] = std::string("The connectivity table specifying for each element the nodes in the coordinates table");
  
  m_coordinates = create_static_component<CLink>("coordinates");
  m_coordinates->add_tag("coordinates");
  
  m_node_list = create_static_component<CList<Uint> >("node_list");
  m_node_list->properties()["brief"] = std::string("The local node indices used by this connectivity specific connectivity table");
  
  CList<Uint>::Ptr global_indices = create_static_component<CList<Uint> >("global_indices");
  global_indices->add_tag("global_element_indices");
  global_indices->properties()["brief"] = std::string("The global element indices (inter processor)");
}

////////////////////////////////////////////////////////////////////////////////

CElements::~CElements()
{
}

////////////////////////////////////////////////////////////////////////////////

void CElements::initialize(const std::string& element_type_name, CTable<Real>& coords_in)
{
  set_element_type(element_type_name);

  const Uint nb_nodes = m_element_type->nb_nodes();
  m_connectivity_table->initialize(nb_nodes);
  
  m_coordinates->link_to(coords_in.get());
}

////////////////////////////////////////////////////////////////////////////////

void CElements::set_element_type(const std::string& etype_name)
{
  m_element_type = create_component_abstract_type<ElementType>( etype_name, etype_name );
  add_static_component( m_element_type );
}
  
//////////////////////////////////////////////////////////////////////////////

CList<Uint>& CElements::update_node_list()
{
  // Assemble all unique node numbers in a set
  std::set<Uint> node_set;
  BOOST_FOREACH(CTable<Uint>::Row row, m_connectivity_table->array())
  BOOST_FOREACH(const Uint node, row)
  {
    node_set.insert(node);
  }
  
  // Copy the set to the node_list
  m_node_list->resize(node_set.size());
  Uint cnt=0;
  BOOST_FOREACH(const Uint node, node_set)
    (*m_node_list)[cnt++] = node;
  
  return *m_node_list;
}

//////////////////////////////////////////////////////////////////////////////

const ElementType& CElements::element_type() const 
{ 
  cf_assert_desc("element_type not initialized", is_not_null(m_element_type));
  return *m_element_type; 
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

CTable<Real>& CElements::coordinates()
{
  return *m_coordinates->get()->as_type<CTable<Real> >();
}

//////////////////////////////////////////////////////////////////////////////

const CTable<Real>& CElements::coordinates() const
{
  return *m_coordinates->get()->as_type<CTable<Real> >();
}
  
//////////////////////////////////////////////////////////////////////////////

CList<Uint>& CElements::node_list()
{
  return *m_node_list;
}

//////////////////////////////////////////////////////////////////////////////

const CList<Uint>& CElements::node_list() const
{
  return *m_node_list;
}

//////////////////////////////////////////////////////////////////////////////

Uint CElements::elements_count() const
{
  return m_connectivity_table->size();
}

//////////////////////////////////////////////////////////////////////////////

void CElements::add_field_elements_link(CElements& field_elements)
{
  CGroup::Ptr field_group = get_child<CGroup>("fields");
  if (!field_group.get())
    field_group = create_component<CGroup>("fields");

  const std::string field_name = field_elements.get_parent()->as_type<CField>()->field_name();
  field_group->create_component<CLink>(field_name)->link_to(field_elements.get());
}

//////////////////////////////////////////////////////////////////////////////

CFieldElements& CElements::get_field_elements(const std::string& field_name)
{
  Component::Ptr all_fields = get_child("fields");
  cf_assert(all_fields.get());
  Component::Ptr field = all_fields->get_child(field_name);
  cf_assert(field.get());
  return *field->as_type<CFieldElements>();
}
  
//////////////////////////////////////////////////////////////////////////////

const CFieldElements& CElements::get_field_elements(const std::string& field_name) const
{
  Component::ConstPtr all_fields = get_child("fields");
  cf_assert(all_fields.get());
  Component::ConstPtr field = all_fields->get_child(field_name);
  cf_assert(field.get());
  return *field->as_type<CFieldElements const>();
}

//////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
