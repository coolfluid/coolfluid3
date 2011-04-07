// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "Common/CreateComponent.hpp"

#include "Mesh/CNodes.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CList.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ComponentBuilder < CNodes, Component, LibMesh > CNodes_Builder;

////////////////////////////////////////////////////////////////////////////////

CNodes::CNodes ( const std::string& name ) :
  Component ( name )
{
  m_coordinates = create_static_component< CTable<Real> >(Mesh::Tags::coordinates());
  m_coordinates->add_tag(Mesh::Tags::coordinates());
  
  m_glb_elem_connectivity = create_static_component< CDynTable<Uint> >("glb_elem_connectivity");
  m_glb_elem_connectivity->add_tag("glb_elem_connectivity");
  
  m_is_ghost = create_static_component< CList<bool> >("is_ghost");
  m_is_ghost->add_tag("is_ghost");
  
  m_global_numbering = create_static_component< CList<Uint> >(Mesh::Tags::global_node_indices());
  m_global_numbering->add_tag(Mesh::Tags::global_node_indices());
  
  add_tag(Mesh::Tags::nodes());
}

////////////////////////////////////////////////////////////////////////////////

CNodes::~CNodes()
{
}

//////////////////////////////////////////////////////////////////////////////

void CNodes::resize(const Uint size)
{
  coordinates().resize(size);
  is_ghost().resize(size);
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
