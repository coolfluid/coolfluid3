// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "Common/CBuilder.hpp"

#include "Mesh/CNodes.hpp"
#include "Mesh/CDynTable.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ComponentBuilder < CNodes, Component, LibMesh > CNodes_Builder;

////////////////////////////////////////////////////////////////////////////////

CNodes::CNodes ( const std::string& name ) :
  FieldGroup ( name )
{
  m_coordinates = create_static_component_ptr< Field >(Mesh::Tags::coordinates());
  m_coordinates->add_tag(Mesh::Tags::coordinates());

  m_glb_elem_connectivity = create_static_component_ptr< CDynTable<Uint> >("glb_elem_connectivity");
  m_glb_elem_connectivity->add_tag("glb_elem_connectivity");

  add_tag(Mesh::Tags::nodes());
}

////////////////////////////////////////////////////////////////////////////////

CNodes::~CNodes()
{
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
