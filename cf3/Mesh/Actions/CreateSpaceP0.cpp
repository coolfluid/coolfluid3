// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/CBuilder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/CBuilder.hpp"
#include "common/OptionT.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/ElementType.hpp"

#include "Mesh/Actions/CreateSpaceP0.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {
namespace Actions {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CreateSpaceP0, CMeshTransformer, LibActions> CreateSpaceP0_Builder;

//////////////////////////////////////////////////////////////////////////////

CreateSpaceP0::CreateSpaceP0( const std::string& name )
: CMeshTransformer(name)
{

  properties()["brief"] = std::string("Create space for FVM shape function");
  properties()["description"] = std::string("The polynomial order \"P\" is configurable, default: P = 0");
}

/////////////////////////////////////////////////////////////////////////////

void CreateSpaceP0::execute()
{

  CMesh& mesh = *m_mesh.lock();

  boost_foreach(CEntities& entities, find_components_recursively<CEntities>(mesh))
  {
    entities.create_space("P0","CF.Mesh.LagrangeP0."+entities.element_type().shape_name());
  }
}

//////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // cf3
