// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/FindComponentss.hpp"

#include "Common/CLink.hpp"
#include "Common/CBuilder.hpp"
#include "Mesh/CNodeFaceCellConnectivity.hpp"
#include "Mesh/CDynTable.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CRegion.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ComponentBuilder < CNodeFaceCellConnectivity , Component, LibMesh > CNodeFaceCellConnectivity_Builder;

////////////////////////////////////////////////////////////////////////////////

CNodeFaceCellConnectivity::CNodeFaceCellConnectivity ( const std::string& name ) : 
  Component(name)
{
  m_nodes = create_static_component<Common::CLink>("nodes");
  m_face_cell_connectivity = create_static_component<CUnifiedData<CFaceCellConnectivity> >("elements");
  m_connectivity = create_static_component<CDynTable<Uint> >("connectivity_table");
  mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void CNodeFaceCellConnectivity::setup(CRegion& region)
{
  add_face_cell_connectivity(find_components_recursively<CFaceCellConnectivity>(region).as_vector());
  build_connectivity();
}

////////////////////////////////////////////////////////////////////////////////

void CNodeFaceCellConnectivity::set_nodes(CNodes& nodes)
{
  m_nodes->link_to(nodes.self());
  m_connectivity->resize(nodes.size());
}

////////////////////////////////////////////////////////////////////////////////

void CNodeFaceCellConnectivity::build_connectivity()
{
  CNodes const& nodes = *m_nodes->follow()->as_ptr<CNodes>();
  
  // Reserve memory in m_connectivity->array()
  std::vector<Uint> connectivity_sizes(nodes.size());
  boost_foreach(CFaceCellConnectivity::ConstPtr face_cell_connectivity, m_face_cell_connectivity->data_components() )
  {
    for (Uint f=0; f<face_cell_connectivity->size(); ++f)
    {
      if ( face_cell_connectivity->is_bdry_face()[f] )
      {
        boost_foreach (const Uint node_idx, face_cell_connectivity->nodes(f))
        {
          ++connectivity_sizes[node_idx];
        }
      }
    }
  }
  index_foreach ( i, CDynTable<Uint>::Row row, m_connectivity->array() )
  {
    row.reserve(connectivity_sizes[i]);
  }
  
  // fill m_connectivity->array()
  
  Uint glb_face_idx(0);
  boost_foreach(CFaceCellConnectivity::ConstPtr face_cell_connectivity, m_face_cell_connectivity->data_components() )
  {
    for (Uint f=0; f<face_cell_connectivity->size(); ++f, ++glb_face_idx)
    {
      if ( face_cell_connectivity->is_bdry_face()[f] )
      {
        boost_foreach (const Uint node_idx, face_cell_connectivity->nodes(f))
        {
          m_connectivity->array()[node_idx].push_back(glb_face_idx);
        }
      }
    }
  }
  
}

////////////////////////////////////////////////////////////////////////////////

CDynTable<Uint>::ConstRow CNodeFaceCellConnectivity::faces(const Uint node_index) const
{
  return (*m_connectivity)[node_index];
}

////////////////////////////////////////////////////////////////////////////////

CUnifiedData<CFaceCellConnectivity>::data_location_type CNodeFaceCellConnectivity::face_location(const Uint unified_face_idx)
{
  return m_face_cell_connectivity->data_location(unified_face_idx);
}

////////////////////////////////////////////////////////////////////////////////

CUnifiedData<CFaceCellConnectivity>::const_data_location_type CNodeFaceCellConnectivity::face_location(const Uint unified_face_idx) const
{
  return m_face_cell_connectivity->data_location(unified_face_idx);
}

////////////////////////////////////////////////////////////////////////////////

boost::tuple<Uint,Uint> CNodeFaceCellConnectivity::face_local_idx(const Uint unified_face_idx) const
{
  return m_face_cell_connectivity->data_local_idx(unified_face_idx);
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
