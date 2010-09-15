// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"
#include "Common/StreamHelpers.hpp"

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CTable.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ObjectProvider < CTable, Component, MeshLib, NB_ARGS_1 >
CTable_Provider ( CTable::type_name() );

////////////////////////////////////////////////////////////////////////////////

CTable::CTable ( const CName& name  ) :
  Component ( name )
{
  BUILD_COMPONENT;
}

std::ostream& operator<<(std::ostream& os, const CTable::ConstRow& row)
{
  print_vector(os, row);
  return os;
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
