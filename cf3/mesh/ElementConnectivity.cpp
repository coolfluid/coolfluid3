// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/OptionT.hpp"
#include "common/FindComponents.hpp"
#include "common/Link.hpp"
#include "common/Group.hpp"
#include "common/Log.hpp"
#include "common/Builder.hpp"


#include "mesh/ElementConnectivity.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshElements.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

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
