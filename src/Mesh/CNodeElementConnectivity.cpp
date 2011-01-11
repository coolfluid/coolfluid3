// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ComponentPredicates.hpp"

#include "Common/CLink.hpp"
#include "Mesh/CNodeElementConnectivity.hpp"
#include "Mesh/CDynTable.hpp"
#include "Mesh/CNodes.hpp"

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
  CNodes const& nodes = *m_nodes->follow()->as_type<CNodes>();
  
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

CDynTable<Uint>::ConstRow CNodeElementConnectivity::elements(const Uint node_index) const
{
  return (*m_connectivity)[node_index];
}

////////////////////////////////////////////////////////////////////////////////

CUnifiedData<CElements>::data_location_type CNodeElementConnectivity::element_location(const Uint unified_elem_idx)
{
  return m_elements->data_location(unified_elem_idx);
}

////////////////////////////////////////////////////////////////////////////////

CUnifiedData<CElements>::const_data_location_type CNodeElementConnectivity::element_location(const Uint unified_elem_idx) const
{
  return m_elements->data_location(unified_elem_idx);
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
