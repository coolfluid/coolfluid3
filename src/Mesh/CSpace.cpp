// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CreateComponent.hpp"
#include "Common/FindComponents.hpp"
#include "Common/CBuilder.hpp"

#include "Mesh/CSpace.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CEntities.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ComponentBuilder < CSpace, Component, LibMesh > CSpace_Builder;

////////////////////////////////////////////////////////////////////////////////

CSpace::CSpace ( const std::string& name ) :
  Component ( name )
{
  properties()["brief"] = std::string("Spaces are other views of CEntities, for instance a higher-order representation");
//  properties()["description"] = std::string("");

  mark_basic();

}

////////////////////////////////////////////////////////////////////////////////

CSpace::~CSpace()
{
}

////////////////////////////////////////////////////////////////////////////////

void CSpace::initialize(const std::string& shape_function_builder_name)
{
  m_shape_function = create_component_abstract_type<ShapeFunction>( shape_function_builder_name, shape_function_builder_name );
  m_shape_function->rename(m_shape_function->derived_type_name());
  add_static_component( m_shape_function );
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
