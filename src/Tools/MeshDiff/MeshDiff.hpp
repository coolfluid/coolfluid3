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
#include "Common/ModuleRegister.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro MeshDiff_API
/// @note build system defines MeshDiff_EXPORTS when compiling MeshDiffTools files
#ifdef MeshDiff_EXPORTS
#   define MeshDiff_API      CF_EXPORT_API
#   define MeshDiff_TEMPLATE
#else
#   define MeshDiff_API      CF_IMPORT_API
#   define MeshDiff_TEMPLATE CF_TEMPLATE_EXTERN
#endif

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

