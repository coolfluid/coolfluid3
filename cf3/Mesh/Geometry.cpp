// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "common/CBuilder.hpp"

#include "mesh/Geometry.hpp"
#include "mesh/CDynTable.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

common::ComponentBuilder < Geometry, Component, LibMesh > Geometry_Builder;

////////////////////////////////////////////////////////////////////////////////

Geometry::Geometry ( const std::string& name ) :
  FieldGroup ( name )
{
  m_coordinates = create_static_component_ptr< Field >(mesh::Tags::coordinates());
  m_coordinates->add_tag(mesh::Tags::coordinates());
  m_coordinates->create_descriptor("coord[vector]");
  m_glb_elem_connectivity = create_static_component_ptr< CDynTable<Uint> >("glb_elem_connectivity");
  m_glb_elem_connectivity->add_tag("glb_elem_connectivity");

  add_tag(mesh::Tags::nodes());
}

////////////////////////////////////////////////////////////////////////////////

Geometry::~Geometry()
{
}

//////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
