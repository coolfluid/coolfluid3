// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CLink.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/CBuilder.hpp"

#include "Mesh/CFieldElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CNodes.hpp"
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
  
}

////////////////////////////////////////////////////////////////////////////////

CFieldElements::~CFieldElements()
{
}

////////////////////////////////////////////////////////////////////////////////

void CFieldElements::initialize(CElements& elements)
{
  m_support = create_static_component<CLink>("support");
  m_support->link_to(elements.follow());
  m_support->add_tag("support");
  
  // Set the shape function
  set_element_type(elements.element_type().element_type_name());
  cf_assert(m_element_type);
  
  m_connectivity_table = create_static_component<CLink>("connectivity_table");
  boost::static_pointer_cast<CLink>(m_connectivity_table)->link_to(elements.connectivity_table().self());
  
  m_nodes = create_static_component<CLink>("nodes");
  m_nodes->add_tag("nodes");
  m_nodes->link_to(elements.nodes().self());
}

////////////////////////////////////////////////////////////////////////////////

void CFieldElements::add_node_based_storage(CTable<Real>& nodal_data)
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
  return *data.follow()->as_type<CTable<Real> >();
}

//////////////////////////////////////////////////////////////////////////////

const CTable<Real>& CFieldElements::data() const
{
  const Component& data = find_component_with_filter(*this,IsComponentTag(m_data_name));
  return *data.follow()->as_type<CTable<Real> const>();
}

//////////////////////////////////////////////////////////////////////////////

CElements& CFieldElements::get_geometry_elements()
{
  return *m_support->follow()->as_type<CElements>();
}

//////////////////////////////////////////////////////////////////////////////

const CElements& CFieldElements::get_geometry_elements() const
{
  return *m_support->follow()->as_type<CElements>();
}

//////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
