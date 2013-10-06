// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_gausslegendre_API_hpp
#define cf3_mesh_gausslegendre_API_hpp

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Mesh_GaussLegendre_API
/// @note build system defines COOLFLUID_MESH_GAUSSLEGENDRE_EXPORTS when compiling SF files
#ifdef COOLFLUID_MESH_GAUSSLEGENDRE_EXPORTS
#   define mesh_gausslegendre_API      CF3_EXPORT_API
#   define mesh_gausslegendre_TEMPLATE
#else
#   define mesh_gausslegendre_API      CF3_IMPORT_API
#   define mesh_gausslegendre_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_gausslegendre_API_hpp
