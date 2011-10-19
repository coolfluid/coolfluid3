// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"

#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/Actions/CBuildVolume.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/Field.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {
namespace Actions {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CBuildVolume, CMeshTransformer, LibActions> CBuildVolume_Builder;

//////////////////////////////////////////////////////////////////////////////

CBuildVolume::CBuildVolume( const std::string& name )
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

std::string CBuildVolume::brief_description() const
{
  return properties().value<std::string>("brief");
}

/////////////////////////////////////////////////////////////////////////////


std::string CBuildVolume::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" +
      properties().value<std::string>("description");
}

/////////////////////////////////////////////////////////////////////////////

void CBuildVolume::execute()
{

  CMesh& mesh = *m_mesh.lock();

  FieldGroup& cells_P0 = mesh.create_space_and_field_group("cells_P0",FieldGroup::Basis::CELL_BASED,"CF.Mesh.LagrangeP0");
  Field& volume = cells_P0.create_field(Mesh::Tags::volume());
  volume.add_tag(Mesh::Tags::volume());

  boost_foreach( CElements& elements, volume.elements_range() )
  {
    RealMatrix coordinates;  elements.allocate_coordinates(coordinates);

    for (Uint cell_idx = 0; cell_idx<elements.size(); ++cell_idx)
    {
      elements.put_coordinates( coordinates, cell_idx );
      volume[cell_idx][0] = elements.element_type().volume( coordinates );
    }
  }
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // cf3
