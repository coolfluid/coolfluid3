// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"
#include "common/Builder.hpp"
#include "common/List.hpp"

#include "mesh/ConnectivityData.hpp"
#include "mesh/Elements.hpp"
#include "mesh/MeshTriangulator.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "LagrangeP1/Quad2D.hpp"
#include "LagrangeP1/Hexa3D.hpp"
#include "LagrangeP1/Quad3D.hpp"
#include "Space.hpp"
#include "Connectivity.hpp"
#include "Field.hpp"

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

  // Remove the quads and hexas
  BOOST_FOREACH(const Handle<Component>& comp, to_remove)
  {
    comp->parent()->remove_component(*comp);
  }

  const Uint nb_procs = PE::Comm::instance().size();
  const Uint rank = PE::Comm::instance().rank();

  // Total number of elements on this rank
  Uint mesh_nb_elems = 0;
  boost_foreach(Elements& elements , find_components_recursively<Elements>(mesh()))
  {
    mesh_nb_elems += elements.size();
  }

  std::vector<Uint> nb_elements_accumulated;
  if(PE::Comm::instance().is_active())
  {
    // Get the total number of elements on each rank
    PE::Comm::instance().all_gather(mesh_nb_elems, nb_elements_accumulated);
  }
  else
  {
    nb_elements_accumulated.push_back(mesh_nb_elems);
  }
  cf3_assert(nb_elements_accumulated.size() == nb_procs);
  // Sum up the values
  for(Uint i = 1; i != nb_procs; ++i)
    nb_elements_accumulated[i] += nb_elements_accumulated[i-1];

  // Offset to start with for this rank
  Uint element_offset = rank == 0 ? 0 : nb_elements_accumulated[rank-1];

  // Update the element ranks and gids
  boost_foreach(Elements& elements , find_components_recursively<Elements>(mesh()))
  {
    const Uint nb_elems = elements.size();
    elements.rank().resize(nb_elems);
    elements.glb_idx().resize(nb_elems);

    for (Uint elem=0; elem != nb_elems; ++elem)
    {
      elements.rank()[elem] = rank;
      elements.glb_idx()[elem] = elem + element_offset;
    }
    element_offset += nb_elems;
  }

  mesh().update_statistics();
  mesh().update_structures();
  mesh().check_sanity();
}

} // mesh
} // cf3
