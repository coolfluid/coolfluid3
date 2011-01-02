// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CLink.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/CBuilder.hpp"

#include "Mesh/CFieldElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CList.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ComponentBuilder < CFieldElements, Component, LibMesh > CFieldElements_Builder;

////////////////////////////////////////////////////////////////////////////////

CFieldElements::CFieldElements ( const std::string& name ) :
  CElements (name)
{
  properties()["element_based"] = false;
  properties()["node_based"] = false;
  
  m_support = create_static_component<CLink>("support");
  m_support->add_tag("support");
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
  m_support->link_to(elements.get());
  
  m_connectivity_table = boost::dynamic_pointer_cast< CTable<Uint> >(elements.connectivity_table().shared_from_this());
  m_used_nodes = boost::dynamic_pointer_cast< CList<Uint> >(elements.used_nodes().shared_from_this());
}

////////////////////////////////////////////////////////////////////////////////

void CFieldElements::add_node_based_storage(CTable<Real>& nodal_data)
{
  // Set the nodal data
  m_data_name = "node_data";
  CLink::Ptr node_data = create_component<CLink>(m_data_name);
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
  CTable<Real>::Ptr elm_data = create_component<CTable<Real> >(m_data_name);
  elm_data->add_tag(m_data_name);
	elm_data->add_tag("field_data");
  properties()["element_based"] = true;
}

//////////////////////////////////////////////////////////////////////////////

CTable<Real>& CFieldElements::data()
{
  Component& data = find_component_with_filter(*this,IsComponentTag(m_data_name));
  return *data.get()->as_type<CTable<Real> >();
}

//////////////////////////////////////////////////////////////////////////////

const CTable<Real>& CFieldElements::data() const
{
  const Component& data = find_component_with_filter(*this,IsComponentTag(m_data_name));
  return *data.get()->as_type<CTable<Real> const>();
}

//////////////////////////////////////////////////////////////////////////////

CElements& CFieldElements::get_geometry_elements()
{
  return *m_support->get()->as_type<CElements>();
}

//////////////////////////////////////////////////////////////////////////////

const CElements& CFieldElements::get_geometry_elements() const
{
  return *m_support->get()->as_type<CElements>();
}

//////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
