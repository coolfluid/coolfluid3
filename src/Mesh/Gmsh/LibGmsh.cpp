// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Mesh/Gmsh/LibGmsh.hpp"

namespace cf3 {
namespace Mesh {
namespace Gmsh {

cf3::common::RegistLibrary<LibGmsh> libGmsh;

////////////////////////////////////////////////////////////////////////////////

void LibGmsh::initiate_impl()
{
}

void LibGmsh::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Gmsh
} // Mesh
} // cf3
