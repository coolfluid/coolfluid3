// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/CBuilder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"

#include "mesh/Actions/CBuildArea.hpp"
#include "mesh/CCells.hpp"
#include "mesh/CRegion.hpp"
#include "mesh/CSpace.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace Actions {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CBuildArea, MeshTransformer, LibActions> CBuildArea_Builder;

//////////////////////////////////////////////////////////////////////////////

CBuildArea::CBuildArea( const std::string& name )
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

std::string CBuildArea::brief_description() const
{
  return properties().value<std::string>("brief");
}

/////////////////////////////////////////////////////////////////////////////


std::string CBuildArea::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" +
      properties().value<std::string>("description");
}

/////////////////////////////////////////////////////////////////////////////

void CBuildArea::execute()
{

  Mesh& mesh = *m_mesh.lock();

  FieldGroup& faces_P0 = mesh.create_space_and_field_group("faces_P0",FieldGroup::Basis::FACE_BASED,"CF.Mesh.LagrangeP0");
  Field& area = faces_P0.create_field(mesh::Tags::area());
  area.add_tag(mesh::Tags::area());

  boost_foreach(CEntities& elements, area.entities_range() )
  {
    RealMatrix coordinates;  elements.allocate_coordinates(coordinates);

    for (Uint cell_idx = 0; cell_idx<elements.size(); ++cell_idx)
    {
      elements.put_coordinates( coordinates, cell_idx );
      area[area.indexes_for_element(elements,cell_idx)[0]][0] = elements.element_type().area( coordinates );
    }
  }
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // mesh
} // cf3
