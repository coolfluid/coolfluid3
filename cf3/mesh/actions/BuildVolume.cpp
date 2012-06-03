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

#include "mesh/actions/BuildVolume.hpp"
#include "mesh/DiscontinuousDictionary.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/Connectivity.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < BuildVolume, MeshTransformer, mesh::actions::LibActions> BuildVolume_Builder;

//////////////////////////////////////////////////////////////////////////////

BuildVolume::BuildVolume( const std::string& name )
: MeshTransformer(name)
{

  properties()["brief"] = std::string("Print information of the mesh");
  std::string desc;
  desc =
  "  Usage: Info \n\n"
  "          Information given: internal mesh hierarchy,\n"
  "      element distribution for each region, and element type";
  properties()["description"] = desc;
}

/////////////////////////////////////////////////////////////////////////////

void BuildVolume::execute()
{

  Mesh& mesh = *m_mesh;

  Dictionary& cells_P0 = *mesh.create_component<DiscontinuousDictionary>("cells_P0");
  boost_foreach(Cells& cells, find_components_recursively<Cells>(mesh.topology()))
    cells.create_space("cf3.mesh.LagrangeP0."+cells.element_type().shape_name(),cells_P0);
  cells_P0.build();         // to tell the dictionary that all spaces have been added
  mesh.update_structures(); // to tell the mesh there is a new dictionary added manually


  Field& volume = cells_P0.create_field("volume");
  volume.add_tag(mesh::Tags::volume());

  boost_foreach( const Handle<Space>& space, volume.spaces() )
  {
    RealMatrix coordinates;  space->support().geometry_space().allocate_coordinates(coordinates);

    const Connectivity& space_connectivity = space->connectivity();
    for (Uint cell_idx = 0; cell_idx<space->size(); ++cell_idx)
    {
      space->support().geometry_space().put_coordinates( coordinates, cell_idx );
      volume[space_connectivity[cell_idx][0]][0] = space->support().element_type().volume( coordinates );
    }
  }

}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
