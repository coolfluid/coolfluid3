// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"
#include "common/Builder.hpp"

#include "mesh/Elements.hpp"
#include "mesh/MeshTriangulator.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "LagrangeP1/Quad2D.hpp"
#include "Space.hpp"
#include "Connectivity.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

common::ComponentBuilder < MeshTriangulator, MeshTransformer, LibMesh> MeshTriangulator_Builder;

MeshTriangulator::MeshTriangulator(const std::string& name) : MeshTransformer(name)
{
}

MeshTriangulator::~MeshTriangulator()
{
}

void MeshTriangulator::execute()
{
  std::vector< Handle<Component> > to_remove;
  // Convert 2D mesh
  BOOST_FOREACH(Elements& quads, find_components_recursively_with_filter<Elements>(mesh(), IsElementType<LagrangeP1::Quad2D>()))
  {
    const Uint nb_quads = quads.size();
    Elements& triags = find_parent_component<Region>(quads).create_elements("cf3.mesh.LagrangeP1.Triag2D", quads.geometry_fields());
    triags.resize(nb_quads*2);
    const Connectivity& quad_conn = quads.geometry_space().connectivity();
    Connectivity& triag_conn = triags.geometry_space().connectivity();
    for(Uint i = 0; i != nb_quads; ++i)
    {
      const Connectivity::ConstRow quad_row = quad_conn[i];
      Connectivity::Row triag_row1 = triag_conn[i*2];
      Connectivity::Row triag_row2 = triag_conn[i*2+1];

      triag_row1[0] = quad_row[0];
      triag_row1[1] = quad_row[1];
      triag_row1[2] = quad_row[2];

      triag_row2[0] = quad_row[2];
      triag_row2[1] = quad_row[3];
      triag_row2[2] = quad_row[0];
    }

    to_remove.push_back(quads.handle());
  }

  BOOST_FOREACH(const Handle<Component>& comp, to_remove)
  {
    comp->parent()->remove_component(*comp);
  }
  
  mesh().update_statistics();
	mesh().check_sanity();
}

} // mesh
} // cf3
