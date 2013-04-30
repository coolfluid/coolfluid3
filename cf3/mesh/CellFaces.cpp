// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/List.hpp"

#include "mesh/CellFaces.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/FaceCellConnectivity.hpp"
#include "common/PropertyList.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

common::ComponentBuilder < CellFaces, Entities, LibMesh > CellFaces_Builder;

////////////////////////////////////////////////////////////////////////////////

CellFaces::CellFaces ( const std::string& name ) :
  Entities ( name )
{
  properties()["brief"] = std::string("Holds information of faces of one element type");
  properties()["description"] = std::string("Container component that stores the element to node connectivity,\n")
  +std::string("a link to node storage, and global numbering unique over all processors");

//  m_cell_connectivity = create_static_component<FaceCellConnectivity>("cell_connectivity");

  add_tag(mesh::Tags::face_entity());
}

////////////////////////////////////////////////////////////////////////////////

CellFaces::~CellFaces()
{
}

////////////////////////////////////////////////////////////////////////////////

//bool CellFaces::is_bdry(const Uint idx) const
//{
//  return m_cell_connectivity->is_bdry_face()[idx];
//}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
