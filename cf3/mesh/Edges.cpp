// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/PropertyList.hpp"

#include "mesh/Edges.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Edges, Entities, LibMesh > Edges_Builder;

////////////////////////////////////////////////////////////////////////////////

Edges::Edges ( const std::string& name ) :
  Elements ( name )
{
  properties()["brief"] = std::string("Holds information of elements of one type");
  properties()["description"] = std::string("Container component that stores the element to node connectivity,\n")
  +std::string("a link to node storage, a list of used nodes, and global numbering unique over all processors");  
  
  add_tag(mesh::Tags::edge_entity());
}

////////////////////////////////////////////////////////////////////////////////

Edges::~Edges()
{
}

//////////////////////////////////////////////////////////////////////////////

void Edges::initialize(const std::string& element_type_name, Dictionary& nodes)
{
  Elements::initialize(element_type_name,nodes);
  cf3_assert(element_type().dimensionality() == element_type().dimension() - 2);
}

//////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
