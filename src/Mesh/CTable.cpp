// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/StreamHelpers.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/CTable.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ComponentBuilder < CTable, Component, LibMesh > CTable_Builder;

////////////////////////////////////////////////////////////////////////////////

CTable::CTable ( const std::string& name  ) :
  Component ( name )
{
  add_tag( type_name() );
}

std::ostream& operator<<(std::ostream& os, const CTable::ConstRow& row)
{
  print_vector(os, row);
  return os;
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
