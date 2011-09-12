// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"

#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/Actions/CBuildArea.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/Field.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {

  using namespace Common;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CBuildArea, CMeshTransformer, LibActions> CBuildArea_Builder;

//////////////////////////////////////////////////////////////////////////////

CBuildArea::CBuildArea( const std::string& name )
: CMeshTransformer(name)
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

  CMesh& mesh = *m_mesh.lock();

  FieldGroup& faces_P0 = mesh.create_space_and_field_group("faces_P0",FieldGroup::Basis::FACE_BASED,"CF.Mesh.LagrangeP0");
  Field& area = faces_P0.create_field(Mesh::Tags::area());
  area.add_tag(Mesh::Tags::area());

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
} // Mesh
} // CF
