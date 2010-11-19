// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Core.hpp"

#include "Mesh/BlockMesh/LibBlockMesh.hpp"

namespace CF {
namespace Mesh {
namespace BlockMesh {

CF::Common::ForceLibRegist<LibBlockMesh> libBlockMesh;

////////////////////////////////////////////////////////////////////////////////

void LibBlockMesh::initiate()
{
}

void LibBlockMesh::terminate()
{
}

////////////////////////////////////////////////////////////////////////////////

} // BlockMesh
} // Mesh
} // CF
