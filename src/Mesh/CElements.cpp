// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

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
  tag_component(this);
}

////////////////////////////////////////////////////////////////////////////////

CElements::~CElements()
{
}

////////////////////////////////////////////////////////////////////////////////

void CElements::initialize(const std::string& element_type_name, CTable<Real>& coords_in)
{
  set_element_type(element_type_name);
  cf_assert(m_element_type);
  const Uint nb_nodes = m_element_type->nb_nodes();
  create_connectivity_table("connectivity_table").initialize(nb_nodes);
  create_node_list("node_list");
  CLink::Ptr coords = create_component_type<CLink>(coords_in.name());
  coords->add_tag("coordinates");
  coords->link_to(coords_in.get());
  
  // Adding global indices for inter-processor connectivity
  CList<Uint>::Ptr global_indices = create_component_type<CList<Uint> >("global_indices");
  global_indices->add_tag("global_element_indices");
}


////////////////////////////////////////////////////////////////////////////////

void CElements::set_element_type(const std::string& etype_name)
{
  m_element_type = create_component_abstract_type<ElementType>( etype_name, etype_name );
  add_static_component( m_element_type );
}

//////////////////////////////////////////////////////////////////////////////

CList<Uint>& CElements::create_node_list( const std::string& name )
{
  CList<Uint>::Ptr node_list = get_child_type< CList<Uint> >(name);
  if (!node_list)
    node_list = create_component_type<CList<Uint> >(name);
    
  node_list->properties()["brief"] = std::string("The local node indices used by this connectivity specific connectivity table");
  return *node_list;
}
  
//////////////////////////////////////////////////////////////////////////////

CList<Uint>& CElements::update_node_list()
{
  // Assemble all unique node numbers in a set
  std::set<Uint> node_set;
  CTable<Uint>& conn_table = connectivity_table();
  BOOST_FOREACH(CTable<Uint>::Row row, conn_table.array())
  BOOST_FOREACH(const Uint node, row)
  node_set.insert(node);
  
  // Copy the set to the node_list
  CList<Uint>& node_list = *get_child_type< CList<Uint> >("node_list");
  node_list.resize(node_set.size());
  Uint cnt=0;
  BOOST_FOREACH(const Uint node, node_set)
    node_list[cnt++] = node;
  
  return node_list;
}

//////////////////////////////////////////////////////////////////////////////

CTable<Uint>& CElements::create_connectivity_table( const std::string& name )
{
  return *create_component_type<CTable<Uint> >(name);
}

//////////////////////////////////////////////////////////////////////////////

CTable<Uint>& CElements::connectivity_table()
{
  Component::Ptr ptr = get_child("connectivity_table");
  cf_assert(ptr);
  CTable<Uint>* tab = dynamic_cast<CTable<Uint>*>(ptr->get().get()); // first get() follows the CLink , second get returns a normal pointer
  cf_assert(tab);
  return *tab;
}

//////////////////////////////////////////////////////////////////////////////

const CTable<Uint>& CElements::connectivity_table() const
{
  Component::ConstPtr ptr = get_child("connectivity_table");
  cf_assert(ptr);
  const CTable<Uint>* tab = dynamic_cast<const CTable<Uint>*>(ptr->get().get());
  cf_assert(tab);
  return *tab;
}

//////////////////////////////////////////////////////////////////////////////

CTable<Real>& CElements::coordinates()
{
  CLink& link = get_component_typed<CLink>(*this,IsComponentTag("coordinates"));
  return *link.get_type<CTable<Real> >();
}

//////////////////////////////////////////////////////////////////////////////

const CTable<Real>& CElements::coordinates() const
{
  const CLink& link = get_component_typed<CLink const>(*this,IsComponentTag("coordinates"));
  return *link.get_type<CTable<Real> const>();

}
  
//////////////////////////////////////////////////////////////////////////////

CList<Uint>& CElements::node_list()
{
  Component::Ptr ptr = get_child("node_list");
  cf_assert(ptr);
  return *ptr->get_type< CList<Uint> >();
}

//////////////////////////////////////////////////////////////////////////////

const CList<Uint>& CElements::node_list() const
{
  Component::ConstPtr ptr = get_child("node_list");
  cf_assert(ptr);
  return *ptr->get_type< CList<Uint> const >();
}

//////////////////////////////////////////////////////////////////////////////

Uint CElements::elements_count() const
{
  return connectivity_table().size();
}

//////////////////////////////////////////////////////////////////////////////

void CElements::add_field_elements_link(CElements& field_elements)
{
  CGroup::Ptr field_group = get_child_type<CGroup>("fields");
  if (!field_group.get())
    field_group = create_component_type<CGroup>("fields");

  const std::string field_name = field_elements.get_parent()->get_type<CField>()->field_name();
  field_group->create_component_type<CLink>(field_name)->link_to(field_elements.get());
}

//////////////////////////////////////////////////////////////////////////////

CFieldElements& CElements::get_field_elements(const std::string& field_name)
{
  Component::Ptr all_fields = get_child("fields");
  cf_assert(all_fields.get());
  Component::Ptr field = all_fields->get_child(field_name);
  cf_assert(field.get());
  return *field->get_type<CFieldElements>();
}
  
//////////////////////////////////////////////////////////////////////////////

const CFieldElements& CElements::get_field_elements(const std::string& field_name) const
{
  Component::ConstPtr all_fields = get_child("fields");
  cf_assert(all_fields.get());
  Component::ConstPtr field = all_fields->get_child(field_name);
  cf_assert(field.get());
  return *field->get_type<CFieldElements const>();
}

//////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
