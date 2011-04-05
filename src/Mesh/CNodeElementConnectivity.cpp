// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/FindComponents.hpp"

#include "Common/CLink.hpp"
#include "Mesh/CNodeElementConnectivity.hpp"
#include "Mesh/CDynTable.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CRegion.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CNodeElementConnectivity::CNodeElementConnectivity ( const std::string& name ) : 
  Component(name)
{
  m_nodes = create_static_component<Common::CLink>("nodes");
  m_elements = create_static_component<CUnifiedData<CElements> >("elements");
  m_connectivity = create_static_component<CDynTable<Uint> >("connectivity_table");
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

void CNodeElementConnectivity::set_nodes(CNodes& nodes)
{
  m_nodes->link_to(nodes.self());
  m_connectivity->resize(nodes.size());
}

////////////////////////////////////////////////////////////////////////////////

void CNodeElementConnectivity::build_connectivity()
{
  set_nodes(elements().data_components()[0]->nodes());
  CNodes const& nodes = *m_nodes->follow()->as_ptr<CNodes>();
  
  // Reserve memory in m_connectivity->array()
  std::vector<Uint> connectivity_sizes(nodes.size());
  boost_foreach(CElements::ConstPtr elements, m_elements->data_components() )
  {
    boost_foreach (CTable<Uint>::ConstRow nodes, elements->connectivity_table().array() )
    {
      boost_foreach (const Uint node_idx, nodes)
      {
        ++connectivity_sizes[node_idx];
      }
    }
  }
  index_foreach ( i, CDynTable<Uint>::Row row, m_connectivity->array() )
  {
    row.reserve(connectivity_sizes[i]);
  }
  
  // fill m_connectivity->array()
  Uint glb_elem_idx = 0;  
  boost_foreach(CElements::ConstPtr elements, m_elements->data_components() )
  {
    boost_foreach (CTable<Uint>::ConstRow nodes, elements->connectivity_table().array() )
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
} // CF
