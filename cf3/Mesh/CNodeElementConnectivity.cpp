// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/FindComponents.hpp"

#include "common/CLink.hpp"
#include "common/CBuilder.hpp"
#include "Mesh/CNodeElementConnectivity.hpp"
#include "Mesh/CDynTable.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/CRegion.hpp"

namespace cf3 {
namespace Mesh {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

ComponentBuilder< CNodeElementConnectivity, Component, LibMesh > CNodeElementConnectivity_Builder;

////////////////////////////////////////////////////////////////////////////////

CNodeElementConnectivity::CNodeElementConnectivity ( const std::string& name ) :
  Component(name)
{
  m_nodes = create_static_component_ptr<common::CLink>(Mesh::Tags::nodes());
  m_elements = create_static_component_ptr<CUnifiedData>("elements");
  m_connectivity = create_static_component_ptr<CDynTable<Uint> >(Mesh::Tags::connectivity_table());
  mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void CNodeElementConnectivity::setup(CRegion& region)
{
  boost_foreach( CElements& elements_comp, find_components_recursively<CElements>(region))
    elements().add(elements_comp);
  build_connectivity();
}

////////////////////////////////////////////////////////////////////////////////

void CNodeElementConnectivity::set_nodes(Geometry& nodes)
{
  m_nodes->link_to(nodes.self());
  m_connectivity->resize(nodes.size());
}

////////////////////////////////////////////////////////////////////////////////

void CNodeElementConnectivity::build_connectivity()
{
  set_nodes(elements().components()[0]->as_type<CElements>().geometry());
  Geometry const& nodes = *m_nodes->follow()->as_ptr<Geometry>();

  // Reserve memory in m_connectivity->array()
  std::vector<Uint> connectivity_sizes(nodes.size());
  boost_foreach(Component::Ptr elements_comp, m_elements->components() )
  {
    CElements& elements = elements_comp->as_type<CElements>();
    boost_foreach (CConnectivity::ConstRow nodes, elements.node_connectivity().array() )
    {
      boost_foreach (const Uint node_idx, nodes)
      {
        ++connectivity_sizes[node_idx];
      }
    }
  }
  Uint i(0);
  boost_foreach (CDynTable<Uint>::Row row, m_connectivity->array() )
  {
    row.reserve(connectivity_sizes[i++]);
  }

  // fill m_connectivity->array()
  Uint glb_elem_idx = 0;
  boost_foreach(Component::Ptr elements_comp, m_elements->components() )
  {
    CElements& elements = elements_comp->as_type<CElements>();
    boost_foreach (CConnectivity::ConstRow nodes, elements.node_connectivity().array() )
    {
      boost_foreach (const Uint node_idx, nodes)
      {
        m_connectivity->array()[node_idx].push_back(glb_elem_idx);
      }
      ++glb_elem_idx;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // cf3
