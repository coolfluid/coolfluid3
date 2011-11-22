// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Space.hpp"
#include "mesh/ElementType.hpp"

#include "mesh/actions/CreateSpaceP0.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CreateSpaceP0, MeshTransformer, mesh::actions::LibActions> CreateSpaceP0_Builder;

//////////////////////////////////////////////////////////////////////////////

CreateSpaceP0::CreateSpaceP0( const std::string& name )
: MeshTransformer(name)
{

  properties()["brief"] = std::string("Create space for FVM shape function");
  properties()["description"] = std::string("The polynomial order \"P\" is configurable, default: P = 0");
}

/////////////////////////////////////////////////////////////////////////////

void CreateSpaceP0::execute()
{

  Mesh& mesh = *m_mesh;

  boost_foreach(Entities& entities, find_components_recursively<Entities>(mesh))
  {
    entities.create_space("P0","cf3.mesh.LagrangeP0."+entities.element_type().shape_name());
  }
}

//////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3
