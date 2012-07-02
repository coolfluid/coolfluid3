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

#include "mesh/actions/BuildArea.hpp"
#include "mesh/DiscontinuousDictionary.hpp"
#include "mesh/Faces.hpp"
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

common::ComponentBuilder < BuildArea, MeshTransformer, mesh::actions::LibActions> BuildArea_Builder;

//////////////////////////////////////////////////////////////////////////////

BuildArea::BuildArea( const std::string& name )
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

void BuildArea::execute()
{

  Mesh& mesh = *m_mesh;


  Dictionary& faces_P0 = *mesh.create_component<DiscontinuousDictionary>("faces_P0");
  boost_foreach(Faces& faces, find_components_recursively<Faces>(mesh.topology()))
    faces.create_space("cf3.mesh.LagrangeP0."+faces.element_type().shape_name(),faces_P0);
  faces_P0.build();         // to tell the dictionary that all spaces have been added
  mesh.update_structures(); // to tell the mesh there is a new dictionary added manually

  Field& area = faces_P0.create_field(mesh::Tags::area());
  area.add_tag(mesh::Tags::area());

  boost_foreach(const Handle<Space>& space, area.spaces() )
  {
    RealMatrix coordinates;  space->support().geometry_space().allocate_coordinates(coordinates);
    const Connectivity& field_connectivity = space->connectivity();
    for (Uint face_idx = 0; face_idx<space->size(); ++face_idx)
    {
      space->support().geometry_space().put_coordinates( coordinates, face_idx );
      area[field_connectivity[face_idx][0]][0] = space->support().element_type().area( coordinates );
    }
  }
}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
