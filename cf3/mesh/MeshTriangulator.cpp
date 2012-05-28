// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"
#include "common/Builder.hpp"

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
  
  // Convert 3D mesh
  
  // Keeps track of the per-tetrahedron indices
  const Uint tetra_indices[5][4] = {
    {0, 1, 2, 5},
    {0, 2, 3, 7},
    {5, 7, 6, 2},
    {4, 7, 5, 0},
    {0, 7, 5, 2}
  };
    
  // local node indices for the triangles that split each side of the hexahedron
  const Uint face_nodes[6][2][3] = {
    {{0, 3, 2}, {2, 1, 0}},
    {{4, 5, 7}, {5, 6, 7}},
    {{0, 1, 5}, {5, 4, 0}},
    {{1, 2, 5}, {2, 6, 5}},
    {{2, 3, 7}, {7, 6, 2}},
    {{0, 4, 7}, {7, 3, 0}}
  };
  
  // We use connectivity information to ensure that quad surface patches are subdivided the same way as the adjacent cell
  boost::shared_ptr<CNodeConnectivity> node_connectivity = allocate_component<CNodeConnectivity>("node_connectivity");
  node_connectivity->initialize(find_components_recursively_with_filter<Elements>(mesh(), IsElementType<LagrangeP1::Quad3D>()));
  
  // Build a mapping between Quad3D and Triag3D element patches
  std::map<Elements const*, Elements*> surface_patches;
  BOOST_FOREACH(Elements& quads, find_components_recursively_with_filter<Elements>(mesh(), IsElementType<LagrangeP1::Quad3D>()))
  {
    Elements& triags = find_parent_component<Region>(quads).create_elements("cf3.mesh.LagrangeP1.Triag3D", quads.geometry_fields());
    triags.resize(quads.size()*2);
    surface_patches[&quads] = &triags;
    to_remove.push_back(quads.handle());
  }
  
  BOOST_FOREACH(Elements& hexas, find_components_recursively_with_filter<Elements>(mesh(), IsElementType<LagrangeP1::Hexa3D>()))
  {
    const Uint nb_hexas = hexas.size();
    Elements& tetras = find_parent_component<Region>(hexas).create_elements("cf3.mesh.LagrangeP1.Tetra3D", hexas.geometry_fields());
    tetras.resize(nb_hexas*5);
    const Connectivity& hexa_conn = hexas.geometry_space().connectivity();
    Connectivity& tetra_conn = tetras.geometry_space().connectivity();
    
    // Connectivity to any adjacent surface patches
    CFaceConnectivity& face_connectivity = *hexas.create_component<CFaceConnectivity>("CellToFaceConnectivity");
    face_connectivity.initialize(*node_connectivity);
    
    for(Uint i = 0; i != nb_hexas; ++i)
    {
      const Connectivity::ConstRow hexa_row = hexa_conn[i];
      
      // Split up the volume cells
      for(Uint j = 0; j != 5; ++j) // loop over the tetras
      {
        Connectivity::Row tetra_row = tetra_conn[i*5+j];
        for(Uint k = 0; k != 4; ++k) // loop over tetra nodes
        {
          tetra_row[k] = hexa_row[tetra_indices[j][k]];
        }
      }
      
      // Split up the surface patches
      for(Uint j = 0; j != 6; ++j)
      {
        if(face_connectivity.has_adjacent_element(i, j))
        {
          const CFaceConnectivity::ElementReferenceT adj_elem = face_connectivity.adjacent_element(i, j);
          Elements& triags = *surface_patches[adj_elem.first];
          Connectivity::Row triag_row1 = triags.geometry_space().connectivity()[adj_elem.second*2];
          Connectivity::Row triag_row2 = triags.geometry_space().connectivity()[adj_elem.second*2+1];
          
          triag_row1[0] = hexa_row[face_nodes[j][0][0]];
          triag_row1[1] = hexa_row[face_nodes[j][0][1]];
          triag_row1[2] = hexa_row[face_nodes[j][0][2]];

          triag_row2[0] = hexa_row[face_nodes[j][1][0]];
          triag_row2[1] = hexa_row[face_nodes[j][1][1]];
          triag_row2[2] = hexa_row[face_nodes[j][1][2]];
        }
      }
    }

    to_remove.push_back(hexas.handle());
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
