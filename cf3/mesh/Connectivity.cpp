// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "mesh/Connectivity.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

common::ComponentBuilder < Connectivity , Component, LibMesh > Connectivity_Builder;

////////////////////////////////////////////////////////////////////////////////

Connectivity::Connectivity ( const std::string& name ) :
  common::Table<Uint>(name)
{
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
