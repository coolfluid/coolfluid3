// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CBuilder.hpp"

#include "Mesh/CFaces.hpp"

namespace cf3 {
namespace Mesh {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CFaces, CEntities, LibMesh > CFaces_Builder;

////////////////////////////////////////////////////////////////////////////////

CFaces::CFaces ( const std::string& name ) :
  CElements ( name )
{
  properties()["brief"] = std::string("Holds information of elements of one type");
  properties()["description"] = std::string("Container component that stores the element to node connectivity,\n")
  +std::string("a link to node storage, a list of used nodes, and global numbering unique over all processors");  
  
  add_tag(Mesh::Tags::face_entity());
}

////////////////////////////////////////////////////////////////////////////////

CFaces::~CFaces()
{
}

//////////////////////////////////////////////////////////////////////////////

void CFaces::initialize(const std::string& element_type_name, Geometry& nodes)
{
  CElements::initialize(element_type_name,nodes);
  cf3_assert(element_type().dimensionality() == element_type().dimension() - 1);
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // cf3
