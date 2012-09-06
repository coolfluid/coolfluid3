// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/PropertyList.hpp"

#include "mesh/actions/ShortestEdge.hpp"
#include "mesh/DiscontinuousDictionary.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/ElementData.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ShortestEdge, MeshTransformer, mesh::actions::LibActions> ShortestEdge_Builder;

//////////////////////////////////////////////////////////////////////////////

ShortestEdge::ShortestEdge( const std::string& name )
: MeshTransformer(name)
{

  properties()["brief"] = std::string("Gets the length of the shortest distance between any 2 nodes in an element");
  properties().add("shortest_length", -1.);
}

/////////////////////////////////////////////////////////////////////////////

void ShortestEdge::execute()
{
  Mesh& mesh = *m_mesh;

  Real shortest_length = 1e20;

  const Field& coords = mesh.geometry_fields().coordinates();

  boost_foreach(mesh::Elements& elements, common::find_components_recursively<mesh::Elements>(mesh.topology()))
  {
    const Table<Uint>& conn = elements.geometry_space().connectivity();
    const Uint nb_elems = conn.size();
    const Uint nb_nodes = conn.row_size();
    RealMatrix element_coords(nb_nodes, coords.row_size());
    for(Uint row_idx = 0; row_idx != nb_elems; ++row_idx)
    {
      fill(element_coords, coords, conn[row_idx]);
      for(Uint i = 1; i != nb_nodes; ++i)
      {
        element_coords.norm();
        const Real l2 = (element_coords.row(0) - element_coords.row(i)).squaredNorm();
        if(l2 < shortest_length)
          shortest_length = l2;
      }
    }
  }

  properties()["shortest_length"] = sqrt(shortest_length);
}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
