// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/FindComponents.hpp"

#include "common/Link.hpp"
#include "common/Builder.hpp"
#include "mesh/Node2FaceCellConnectivity.hpp"
#include "common/DynTable.hpp"
#include "mesh/SpaceFields.hpp"
#include "mesh/Region.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

common::ComponentBuilder < Node2FaceCellConnectivity , Component, LibMesh > Node2FaceCellConnectivity_Builder;

////////////////////////////////////////////////////////////////////////////////

Node2FaceCellConnectivity::Node2FaceCellConnectivity ( const std::string& name ) :
  Component(name)
{
  m_nodes = create_static_component_ptr<common::Link>(mesh::Tags::nodes());
  m_face_cell_connectivity = create_static_component_ptr<UnifiedData>("elements");
  m_connectivity = create_static_component_ptr<DynTable<Uint> >(mesh::Tags::connectivity_table());
  mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void Node2FaceCellConnectivity::setup(Region& region)
{
  boost_foreach(FaceCellConnectivity& face2cells , find_components_recursively<FaceCellConnectivity>(region))
    face_cell_connectivity().add(face2cells);
  build_connectivity();
}

////////////////////////////////////////////////////////////////////////////////

void Node2FaceCellConnectivity::set_nodes(SpaceFields& nodes)
{
  m_nodes->link_to(nodes.self());
  m_connectivity->resize(nodes.size());
}

////////////////////////////////////////////////////////////////////////////////

void Node2FaceCellConnectivity::build_connectivity()
{
  SpaceFields const& nodes = *m_nodes->follow()->as_ptr<SpaceFields>();

  // Reserve memory in m_connectivity->array()
  std::vector<Uint> connectivity_sizes(nodes.size());
  boost_foreach(boost::weak_ptr<Component> face_cell_connectivity_comp, m_face_cell_connectivity->components() )
  {
    const FaceCellConnectivity& face_cell_connectivity = face_cell_connectivity_comp.lock()->as_type<FaceCellConnectivity>();
    for (Uint f=0; f<face_cell_connectivity.size(); ++f)
    {
      if ( face_cell_connectivity.is_bdry_face()[f] )
      {
        boost_foreach (const Uint node_idx, face_cell_connectivity.face_nodes(f))
        {
          ++connectivity_sizes[node_idx];
        }
      }
    }
  }
  Uint i(0);
  boost_foreach (DynTable<Uint>::Row row, m_connectivity->array() )
  {
    row.reserve(connectivity_sizes[i++]);
  }

  // fill m_connectivity->array()

  Uint glb_face_idx(0);
  boost_foreach(boost::weak_ptr<Component> face_cell_connectivity_comp, m_face_cell_connectivity->components() )
  {
    const FaceCellConnectivity& face_cell_connectivity = face_cell_connectivity_comp.lock()->as_type<FaceCellConnectivity>();
    for (Uint f=0; f<face_cell_connectivity.size(); ++f, ++glb_face_idx)
    {
      if ( face_cell_connectivity.is_bdry_face()[f] )
      {
        boost_foreach (const Uint node_idx, face_cell_connectivity.face_nodes(f))
        {
          m_connectivity->array()[node_idx].push_back(glb_face_idx);
        }
      }
    }
  }

}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
