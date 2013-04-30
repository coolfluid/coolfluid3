// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Tools_MeshDiff_Tools_hpp
#define cf3_Tools_MeshDiff_Tools_hpp

#include "mesh/Mesh.hpp"

#include "Tools/Testing/Difference.hpp"

#include "Tools/MeshDiff/LibMeshDiff.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Tools {
namespace MeshDiff {

////////////////////////////////////////////////////////////////////////////////

/// Calculates the difference between two meshes
bool MeshDiff_API diff( const cf3::mesh::Mesh& a, const cf3::mesh::Mesh& b, const cf3::Uint max_ulps);

////////////////////////////////////////////////////////////////////////////////

} // MeshDiff
} // Tools
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Tools_MeshDiff_Tools_hpp

