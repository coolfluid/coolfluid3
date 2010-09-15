// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Factory.hpp"
#include "Common/CLink.hpp"
#include "Common/CGroup.hpp"
#include "Common/ObjectProvider.hpp"

#include "Mesh/CElements.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CTable.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ObjectProvider < CElements, Component, MeshLib, NB_ARGS_1 >
CElements_Provider ( CElements::type_name() );

////////////////////////////////////////////////////////////////////////////////

CElements::CElements ( const CName& name ) :
  Component ( name )
{
  BUILD_COMPONENT;
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

  CLink::Ptr coords = create_component_type<CLink>(coords_in.name());
  coords->add_tag("coordinates");
  coords->link_to(coords_in.get());
}


////////////////////////////////////////////////////////////////////////////////

void CElements::set_element_type(const std::string& etype_name)
{
  Common::SafePtr< ElementType::PROVIDER > prov =
      Factory<ElementType>::instance().getProvider( etype_name );

  m_element_type = prov->create();
}

//////////////////////////////////////////////////////////////////////////////

CTable& CElements::create_connectivity_table( const CName& name )
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

CFieldElements& CElements::get_field_elements(const CName& field_name)
{
  Component::Ptr all_fields = get_child("fields");
  cf_assert(all_fields.get());
  Component::Ptr field = all_fields->get_child(field_name);
  cf_assert(field.get());
  return *field->get_type<CFieldElements>();
}
  
//////////////////////////////////////////////////////////////////////////////

const CFieldElements& CElements::get_field_elements(const CName& field_name) const
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
