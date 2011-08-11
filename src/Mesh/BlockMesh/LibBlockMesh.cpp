// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Mesh/BlockMesh/LibBlockMesh.hpp"

namespace CF {
namespace Mesh {
namespace BlockMesh {

CF::Common::RegistLibrary<LibBlockMesh> libBlockMesh;

////////////////////////////////////////////////////////////////////////////////

void LibBlockMesh::initiate_impl()
{
}

void LibBlockMesh::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // BlockMesh
} // Mesh
} // CF
