// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Mesh/CElements.hpp"

#include "FVM/Core/GhostCells.hpp"

namespace CF {
namespace FVM {
namespace Core {

using namespace Common;
using namespace Mesh;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < GhostCells, CEntities, LibCore > GhostCells_Builder;

////////////////////////////////////////////////////////////////////////////////

GhostCells::GhostCells ( const std::string& name ) :
  CCells ( name )
{
  properties()["brief"] = std::string("Holds information of elements of one type");
  properties()["description"] = std::string("Container component that stores the element to node connectivity,\n")
  +std::string("a link to node storage, a list of used nodes, and global numbering unique over all processors");  
}

////////////////////////////////////////////////////////////////////////////////

GhostCells::~GhostCells()
{
}

//////////////////////////////////////////////////////////////////////////////

void GhostCells::initialize(const std::string& element_type_name, CNodes& nodes)
{
  Mesh::CElements::initialize(element_type_name,nodes);
  cf_assert(element_type().dimensionality() == 0);
  // ghost cell dimensionality is zero since it is a point
}

//////////////////////////////////////////////////////////////////////////////

} // Core
} // FVM
} // CF
