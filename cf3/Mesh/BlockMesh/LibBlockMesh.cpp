// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"

#include "mesh/Blockmesh/LibBlockMesh.hpp"

namespace cf3 {
namespace mesh {
namespace BlockMesh {

cf3::common::RegistLibrary<LibBlockMesh> libBlockMesh;

////////////////////////////////////////////////////////////////////////////////

void LibBlockMesh::initiate_impl()
{
}

void LibBlockMesh::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // BlockMesh
} // mesh
} // cf3
