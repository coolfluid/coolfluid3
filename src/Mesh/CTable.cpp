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

Common::ComponentBuilder < CTable<bool>, Component, LibMesh > CTable_bool_Builder;

Common::ComponentBuilder < CTable<Uint>, Component, LibMesh > CTable_Uint_Builder;

Common::ComponentBuilder < CTable<int>, Component, LibMesh >  CTable_int_Builder;

Common::ComponentBuilder < CTable<Real>, Component, LibMesh > CTable_Real_Builder;

Common::ComponentBuilder < CTable<std::string>, Component, LibMesh > CTable_string_Builder;

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const CTable<bool>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, const CTable<Uint>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, const CTable<int>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, const CTable<Real>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, const CTable<std::string>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
