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

std::string BuildArea::brief_description() const
{
  return properties().value<std::string>("brief");
}

/////////////////////////////////////////////////////////////////////////////


std::string BuildArea::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" +
      properties().value<std::string>("description");
}

/////////////////////////////////////////////////////////////////////////////

void BuildArea::execute()
{

  Mesh& mesh = *m_mesh;

  SpaceFields& faces_P0 = mesh.create_space_and_field_group("faces_P0",SpaceFields::Basis::FACE_BASED,"cf3.mesh.LagrangeP0");
  Field& area = faces_P0.create_field(mesh::Tags::area());
  area.add_tag(mesh::Tags::area());

  boost_foreach(const Handle<Entities>& elements_handle, area.entities_range() )
  {
    Entities& elements = *elements_handle;
    RealMatrix coordinates;  elements.allocate_coordinates(coordinates);
    Connectivity& field_connectivity = area.space(elements).connectivity();
    for (Uint cell_idx = 0; cell_idx<elements.size(); ++cell_idx)
    {
      elements.put_coordinates( coordinates, cell_idx );
      area[field_connectivity[cell_idx][0]][0] = elements.element_type().area( coordinates );
    }
  }
}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
