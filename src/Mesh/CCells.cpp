// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Mesh/CCells.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CCells, CEntities, LibMesh > CCells_Builder;

////////////////////////////////////////////////////////////////////////////////

CCells::CCells ( const std::string& name ) :
  CElements ( name )
{
  properties()["brief"] = std::string("Holds information of elements of one type");
  properties()["description"] = std::string("Container component that stores the element to node connectivity,\n")
  +std::string("a link to node storage, a list of used nodes, and global numbering unique over all processors");  
}

////////////////////////////////////////////////////////////////////////////////

CCells::~CCells()
{
}

//////////////////////////////////////////////////////////////////////////////

void CCells::initialize(const std::string& element_type_name, CNodes& nodes)
{
  CElements::initialize(element_type_name,nodes);
  cf_assert(element_type().dimensionality() == element_type().dimension());
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
