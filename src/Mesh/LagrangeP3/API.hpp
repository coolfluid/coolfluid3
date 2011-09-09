// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LagrangeP3_API_hpp
#define CF_Mesh_LagrangeP3_API_hpp

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Mesh_LagrangeP3_API
/// @note build system defines COOLFLUID_MESH_LAGRANGEP3_EXPORTS when compiling SF files
#ifdef COOLFLUID_MESH_LAGRANGEP3_EXPORTS
#   define Mesh_LagrangeP3_API      CF_EXPORT_API
#   define Mesh_LagrangeP3_TEMPLATE
#else
#   define Mesh_LagrangeP3_API      CF_IMPORT_API
#   define Mesh_LagrangeP3_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_LagrangeP3_API_hpp
