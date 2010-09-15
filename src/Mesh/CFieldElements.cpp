// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CLink.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CFieldElements.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CTable.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CFieldElements::CFieldElements ( const CName& name ) :
  CElements (name)
{
  BUILD_COMPONENT;
  configure_property("element_based", false);
  configure_property("node_based", false);
}

////////////////////////////////////////////////////////////////////////////////

CFieldElements::~CFieldElements()
{
}

////////////////////////////////////////////////////////////////////////////////

void CFieldElements::initialize(CElements& elements)
{
  // Set the shape function
  set_element_type(elements.element_type().getElementTypeName());
  cf_assert(m_element_type);

  // create a link to the geometry elements.
  CLink::Ptr support = create_component_type<CLink>("support");
  support->link_to(elements.get());
  support->add_tag("support");

  // create the connectivity table as a CLink to another one.
  CLink::Ptr connectivity_table = create_component_type<CLink>(elements.connectivity_table().name());
  connectivity_table->link_to(elements.connectivity_table().get());

}

////////////////////////////////////////////////////////////////////////////////

void CFieldElements::add_node_based_storage(CArray& nodal_data)
{
  // Set the nodal data
  m_data_name = "node_data";
  CLink::Ptr node_data = create_component_type<CLink>(m_data_name);
  node_data->add_tag(m_data_name);
  node_data->link_to(nodal_data.get());
  configure_property("node_based", true);
}

////////////////////////////////////////////////////////////////////////////////

void CFieldElements::add_element_based_storage()
{
  // Create elemental data
  m_data_name = "element_data";
  CArray::Ptr elm_data = create_component_type<CArray>(m_data_name);
  elm_data->add_tag(m_data_name);
  configure_property("element_based", true);
}

//////////////////////////////////////////////////////////////////////////////

CArray& CFieldElements::data()
{
  Component& data = get_component(*this,IsComponentTag(m_data_name));
  return *data.get_type<CArray>();
}

//////////////////////////////////////////////////////////////////////////////

const CArray& CFieldElements::data() const
{
  const Component& data = get_component(*this,IsComponentTag(m_data_name));
  return *data.get_type<CArray const>();
}

//////////////////////////////////////////////////////////////////////////////

CElements& CFieldElements::get_geometry_elements()
{
  Component& geometry_elements = get_component(*this,IsComponentTag("support"));
  return *geometry_elements.get_type<CElements>();
}

//////////////////////////////////////////////////////////////////////////////

const CElements& CFieldElements::get_geometry_elements() const
{
  const Component& geometry_elements = get_component(*this,IsComponentTag("support"));
  return *geometry_elements.get_type<CElements const>();
}

//////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
