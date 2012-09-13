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

#include "common/PE/Comm.hpp"

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
  const Uint dimension = mesh.dimension();

  RealVector min_h(3);
  min_h.setConstant(1e50);

  const Field& coords = mesh.geometry_fields().coordinates();

  boost_foreach(mesh::Elements& elements, common::find_components_recursively<mesh::Elements>(mesh.topology()))
  {
    const ShapeFunction& sf = elements.geometry_space().shape_function();
    if(sf.dimensionality() != dimension)
      continue;

    const Table<Uint>& conn = elements.geometry_space().connectivity();
    const Uint nb_elems = conn.size();
    const Uint nb_nodes = conn.row_size();
    RealMatrix element_coords(nb_nodes, dimension);

    const Real xi_min = (sf.shape() == cf3::mesh::GeoShape::TRIAG || sf.shape() == cf3::mesh::GeoShape::TETRA) ? 0. : -1.;
    const Real xi_max = 1.;
    RealVector origin(dimension); origin.setConstant(xi_min);
    RealMatrix end_coords(dimension, dimension);
    end_coords.setConstant(xi_min);
    end_coords.diagonal().setConstant(xi_max);

    RealRowVector sf_origin_value(sf.nb_nodes());
    RealRowVector sf_other_value(sf.nb_nodes());
    RealRowVector origin_real;
    
    for(Uint row_idx = 0; row_idx != nb_elems; ++row_idx)
    {
      fill(element_coords, coords, conn[row_idx]);
      sf.compute_value(origin, sf_origin_value);
      origin_real = sf_origin_value*element_coords;
      
      for(Uint i = 0; i != dimension; ++i)
      {
        sf.compute_value(end_coords.row(i), sf_other_value);
        const Real h = (sf_other_value*element_coords - origin_real).squaredNorm();
        if(h < min_h[i])
          min_h[i] = h;
      }
    }
  }

  RealVector global_min_h = min_h;
  
  if(PE::Comm::instance().is_active())
  {
    PE::Comm::instance().all_reduce(PE::min(), min_h.data(), 3, global_min_h.data());
  }
  
  properties()["h_xi"] = sqrt(global_min_h[0]);
  properties()["h_eta"] = sqrt(global_min_h[1]);
  properties()["h_zeta"] = sqrt(global_min_h[2]);
}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
