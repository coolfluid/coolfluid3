// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CLink.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CFieldElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CList.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CFieldElements::CFieldElements ( const std::string& name ) :
  CElements (name)
{
   

  properties()["element_based"] = false;
  properties()["node_based"] = false;
}

////////////////////////////////////////////////////////////////////////////////

CFieldElements::~CFieldElements()
{
}

////////////////////////////////////////////////////////////////////////////////

void CFieldElements::initialize(CElements& elements)
{
  // Set the shape function
  set_element_type(elements.element_type().element_type_name());
  cf_assert(m_element_type);

  // create a link to the geometry elements.
  CLink::Ptr support = create_component_type<CLink>("support");
  support->link_to(elements.get());
  support->add_tag("support");
  
  m_connectivity_table = boost::dynamic_pointer_cast< CTable<Uint> >(elements.connectivity_table().shared_from_this());
  m_node_list = boost::dynamic_pointer_cast< CList<Uint> >(elements.node_list().shared_from_this());
}

////////////////////////////////////////////////////////////////////////////////

void CFieldElements::add_node_based_storage(CTable<Real>& nodal_data)
{
  // Set the nodal data
  m_data_name = "node_data";
  CLink::Ptr node_data = create_component_type<CLink>(m_data_name);
  nodal_data.add_tag(m_data_name);
	node_data->add_tag(m_data_name);
  node_data->link_to(nodal_data.get());
  properties()["node_based"] = true;
}

////////////////////////////////////////////////////////////////////////////////

void CFieldElements::add_element_based_storage()
{
  // Create elemental data
  m_data_name = "element_data";
  CTable<Real>::Ptr elm_data = create_component_type<CTable<Real> >(m_data_name);
  elm_data->add_tag(m_data_name);
	elm_data->add_tag("field_data");
  properties()["element_based"] = true;
}

//////////////////////////////////////////////////////////////////////////////

CTable<Real>& CFieldElements::data()
{
  Component& data = get_component(*this,IsComponentTag(m_data_name));
  return *data.as_type<CTable<Real> >();
}

//////////////////////////////////////////////////////////////////////////////

const CTable<Real>& CFieldElements::data() const
{
  const Component& data = get_component(*this,IsComponentTag(m_data_name));
  return *data.as_type<CTable<Real> const>();
}

//////////////////////////////////////////////////////////////////////////////

CElements& CFieldElements::get_geometry_elements()
{
  Component& geometry_elements = get_component(*this,IsComponentTag("support"));
  return *geometry_elements.as_type<CElements>();
}

//////////////////////////////////////////////////////////////////////////////

const CElements& CFieldElements::get_geometry_elements() const
{
  const Component& geometry_elements = get_component(*this,IsComponentTag("support"));
  return *geometry_elements.as_type<CElements const>();
}

//////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
