// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/FindComponents.hpp"
#include "common/DynTable.hpp"
#include "common/Link.hpp"
#include "common/Builder.hpp"

#include "mesh/NodeElementConnectivity.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Region.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Space.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

ComponentBuilder< NodeElementConnectivity, Component, LibMesh > NodeElementConnectivity_Builder;

////////////////////////////////////////////////////////////////////////////////

NodeElementConnectivity::NodeElementConnectivity ( const std::string& name ) :
  Component(name)
{
  m_nodes = create_static_component<common::Link>(mesh::Tags::nodes());
  m_elements = create_static_component<UnifiedData>("elements");
  m_connectivity = create_static_component<DynTable<Uint> >(mesh::Tags::connectivity_table());
  mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void NodeElementConnectivity::setup(Region& region)
{
  m_connectivity->resize(0);
  elements().reset();
  boost_foreach( Entities& elements_comp, find_components_recursively<Entities>(region))
    elements().add(elements_comp);
  if (elements().size() == 0)
  {
    throw InvalidStructure(FromHere(), region.uri().string()+" does not contain any Entities");
  }
  build_connectivity();
}

////////////////////////////////////////////////////////////////////////////////

void NodeElementConnectivity::set_nodes(Dictionary& nodes)
{
  m_nodes->link_to(nodes);
  m_connectivity->resize(nodes.size());
}

////////////////////////////////////////////////////////////////////////////////

void NodeElementConnectivity::build_connectivity()
{
  cf3_assert(elements().size());
  cf3_assert(elements().components().size());
  cf3_assert(elements().components()[0]);
  set_nodes(Handle<Entities>(elements().components()[0])->geometry_fields());
  cf3_assert(m_nodes);
  cf3_assert(m_nodes->follow());
  Dictionary const& nodes = *Handle<Dictionary>(m_nodes->follow());

  // Reserve memory in m_connectivity->array()
  std::vector<Uint> connectivity_sizes(nodes.size());
  boost_foreach(Handle<Component> elements_comp, m_elements->components() )
  {
    Entities& elements = dynamic_cast<Entities&>(*elements_comp);
    boost_foreach (Connectivity::ConstRow elem_nodes, elements.geometry_space().connectivity().array() )
    {
      boost_foreach (const Uint node_idx, elem_nodes)
      {
        cf3_assert(node_idx<nodes.size());
        ++connectivity_sizes[node_idx];
      }
    }
  }
  Uint i(0);
  boost_foreach (DynTable<Uint>::Row row, m_connectivity->array() )
  {
    row.reserve(connectivity_sizes[i++]);
  }

  // fill m_connectivity->array()
  Uint glb_elem_idx = 0;
  boost_foreach(Handle<Component> elements_comp, m_elements->components() )
  {
    Entities& elements = dynamic_cast<Entities&>(*elements_comp);
    boost_foreach (Connectivity::ConstRow elem_nodes, elements.geometry_space().connectivity().array() )
    {
      boost_foreach (const Uint node_idx, elem_nodes)
      {
        m_connectivity->array()[node_idx].push_back(glb_elem_idx);
      }
      ++glb_elem_idx;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
