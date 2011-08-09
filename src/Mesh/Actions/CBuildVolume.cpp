// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
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

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {

  using namespace Common;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CBuildVolume, CMeshTransformer, LibActions> CBuildVolume_Builder;

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

  Field& volume_field = mesh.create_field(Mesh::Tags::volume(),Field::Basis::CELL_BASED,"P0","volume[1]");
  volume_field.add_tag(Mesh::Tags::volume());
  CScalarFieldView volume("volume_view");
  volume.set_field(volume_field);

  boost_foreach( CCells& elements, find_components_recursively<CCells>(mesh.topology()) )
  {
    volume.set_elements(elements.as_ptr<CEntities>());

    RealMatrix coordinates;  elements.allocate_coordinates(coordinates);

    for (Uint cell_idx = 0; cell_idx<elements.size(); ++cell_idx)
    {
      elements.put_coordinates( coordinates, cell_idx );
      volume[cell_idx] = elements.element_type().compute_volume( coordinates );
    }
  }
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
