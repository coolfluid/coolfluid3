// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include "common/Builder.hpp"
#include "common/Foreach.hpp"
#include "common/Table.hpp"

#include "mesh/ElementConnectivity.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

// FIRST register the value_type to be used in common::Table<ValueT>
RegistTypeInfo<Entity,LibMesh> regist_ElementConnectivity_ValueT;
// THEN create the builder that assumes the ValueT is already registered
common::ComponentBuilder < ElementConnectivity , Component, LibMesh > ElementConnectivity_Builder;

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const ElementConnectivity::ConstRow row)
{
  for (Uint i=0; i<row.size()-1; ++i)
    os << row[i] << " ";
  os << row[row.size()-1];
  return os;
}

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const ElementConnectivity& table)
{
  if (table.size())
    os << "\n";
  Uint i=0;
  boost_foreach(ElementConnectivity::ConstRow row, table.array())
  {
    os << "  " << i++ << ":  " << row << "\n";
  }
  return os;
}


////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
