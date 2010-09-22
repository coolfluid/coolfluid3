// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Tools_MeshDiff_Tools_hpp
#define CF_Tools_MeshDiff_Tools_hpp

#include "Mesh/CMesh.hpp"

#include "Tools/Testing/Difference.hpp"

#include "Common/ExportAPI.hpp"
#include "Common/LibraryRegister.hpp"

#include "Tools/MeshDiff/LibMeshDiff.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace MeshDiff {

////////////////////////////////////////////////////////////////////////////////

/// Calculates the difference between two meshes
bool MeshDiff_API diff( const CF::Mesh::CMesh& a, const CF::Mesh::CMesh& b, const CF::Uint max_ulps);

////////////////////////////////////////////////////////////////////////////////

} // MeshDiff
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Tools_MeshDiff_Tools_hpp

