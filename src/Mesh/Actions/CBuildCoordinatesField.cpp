// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/StreamHelpers.hpp"

#include "Mesh/Actions/CBuildCoordinatesField.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CMesh.hpp"

#include "Math/Functions.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {

  using namespace Common;
  using namespace Math::Functions;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CBuildCoordinatesField, CMeshTransformer, LibActions> CBuildCoordinatesField_Builder;

//////////////////////////////////////////////////////////////////////////////

CBuildCoordinatesField::CBuildCoordinatesField( const std::string& name )
: CMeshTransformer(name)
{

	properties()["brief"] = std::string("Create a coordinate field in the mesh topology");
	std::string desc;
	desc =
	"  Usage: CBuildCoordinatesField \n\n"
	"          Information given: internal mesh hierarchy,\n"
	"      element distribution for each region, and element type";
	properties()["description"] = desc;
}

/////////////////////////////////////////////////////////////////////////////

std::string CBuildCoordinatesField::brief_description() const
{
  return properties().value<std::string>("brief");
}

/////////////////////////////////////////////////////////////////////////////


std::string CBuildCoordinatesField::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" +
      properties().value<std::string>("description");
}

/////////////////////////////////////////////////////////////////////////////

void CBuildCoordinatesField::execute()
{

  CMesh& mesh = *m_mesh.lock();

  std::vector<std::string> names(1);
  std::vector<Uint> sizes(1);

  CField& coordinates = *mesh.create_component_ptr<CField>("coordinates");
  names[0] = "coordinates";
  sizes[0] = mesh.nodes().coordinates().row_size();
  coordinates.get_child_ptr("topology")->as_ptr<CLink>()->link_to(mesh.topology().self());
  coordinates.configure_option("VarNames",names);
  coordinates.configure_option("VarSizes",sizes);
  coordinates.configure_option("FieldType",std::string("NodeBased"));
  coordinates.create_data_storage();

  CNodes& nodes = mesh.nodes();
  Uint data_idx(0);
  boost_foreach(const Uint node_idx, coordinates.used_nodes().array())
  {
    coordinates[data_idx] = nodes.coordinates()[node_idx];
    ++data_idx;
  }

}

//////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF
