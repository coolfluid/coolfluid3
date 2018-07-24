// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Option.hpp"
#include "common/OptionList.hpp"
#include "common/List.hpp"

#include "mesh/ConnectivityData.hpp"
#include "mesh/DiscontinuousDictionary.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/Functions.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/ElementData.hpp"

#include "mesh/LagrangeP1/Triag2D.hpp"
#include "mesh/LagrangeP1/Quad2D.hpp"

#include "SurfaceToVolumeConnectivity.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < SurfaceToVolumeConnectivity, MeshTransformer, mesh::actions::LibActions> SurfaceToVolumeConnectivity_Builder;

////////////////////////////////////////////////////////////////////////////////

SurfaceToVolumeConnectivity::SurfaceToVolumeConnectivity(const std::string& name) : MeshTransformer(name)
{
}

void SurfaceToVolumeConnectivity::execute()
{
  Mesh& mesh = *m_mesh;
  const Uint nb_nodes = mesh.geometry_fields().size();

  if(mesh.get_child("volume_node_connectivity") != nullptr)
  {
    mesh.remove_component("volume_node_connectivity");
  }
  auto volume_node_connectivity = mesh.create_component<NodeConnectivity>("volume_node_connectivity");
  std::vector< Handle<Entities const> > volume_entities;
  for(const mesh::Elements& elements : common::find_components_recursively_with_filter<mesh::Elements>(mesh, IsElementsVolume()))
  {
    volume_entities.push_back(elements.handle<Entities const>());
  }
  volume_node_connectivity->initialize(nb_nodes, volume_entities);

  for(mesh::Elements& wall_entity : common::find_components_recursively_with_filter<mesh::Elements>(mesh, IsElementsSurface()))
  {
    if(wall_entity.get_child("wall_face_connectivity") != nullptr)
    {
      wall_entity.remove_component("wall_face_connectivity");
    }
    wall_entity.create_component<FaceConnectivity>("wall_face_connectivity")->initialize(*volume_node_connectivity);
  }
}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
