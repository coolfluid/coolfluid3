// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLib.hpp"

#include "Mesh/LibMesh.hpp"

namespace CF {
namespace Mesh {

CF::Common::ForceLibRegist<LibMesh> libMesh;

////////////////////////////////////////////////////////////////////////////////

void LibMesh::initiate()
{
}

void LibMesh::terminate()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
