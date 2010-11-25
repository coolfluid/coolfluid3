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
#include "Mesh/CArray.hpp"
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
  BuildComponent<none>().build(this);
}

////////////////////////////////////////////////////////////////////////////////

CElements::~CElements()
{
}

////////////////////////////////////////////////////////////////////////////////

void CElements::initialize(const std::string& element_type_name, CArray& coords_in)
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
  CTable& ctable = connectivity_table();
  BOOST_FOREACH(CTable::Row row, ctable.array())
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

CTable& CElements::create_connectivity_table( const std::string& name )
{
  return *create_component_type<CTable>(name);
}

//////////////////////////////////////////////////////////////////////////////

CTable& CElements::connectivity_table()
{
  Component::Ptr ptr = get_child("connectivity_table");
  cf_assert(ptr);
  CTable* tab = dynamic_cast<CTable*>(ptr->get().get()); // first get() follows the CLink , second get returns a normal pointer
  cf_assert(tab);
  return *tab;
}

//////////////////////////////////////////////////////////////////////////////

const CTable& CElements::connectivity_table() const
{
  Component::ConstPtr ptr = get_child("connectivity_table");
  cf_assert(ptr);
  const CTable* tab = dynamic_cast<const CTable*>(ptr->get().get());
  cf_assert(tab);
  return *tab;
}

//////////////////////////////////////////////////////////////////////////////

CArray& CElements::coordinates()
{
  CLink& link = get_component_typed<CLink>(*this,IsComponentTag("coordinates"));
  return *link.get_type<CArray>();
}

//////////////////////////////////////////////////////////////////////////////

const CArray& CElements::coordinates() const
{
  const CLink& link = get_component_typed<CLink const>(*this,IsComponentTag("coordinates"));
  return *link.get_type<CArray const>();

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
