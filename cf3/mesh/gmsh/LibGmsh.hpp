// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_LibGmsh_hpp
#define cf3_LibGmsh_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro gmsh_API
/// @note build system defines COOLFLUID_GMSH_EXPORTS when compiling gmsh files
#ifdef COOLFLUID_GMSH_EXPORTS
#   define gmsh_API      CF3_EXPORT_API
#   define gmsh_TEMPLATE
#else
#   define gmsh_API      CF3_IMPORT_API
#   define gmsh_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
  
/// @brief Library for I/O of the gmsh format
namespace gmsh {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the gmshtral mesh format operations
/// @author Willem Deconinck
class gmsh_API LibGmsh :
    public common::Library
{
public:

  
  

  /// Constructor
  LibGmsh ( const std::string& name) : common::Library(name) {   }

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.mesh.gmsh"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "gmsh"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the gmsh mesh format operations.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibGmsh"; }
}; // end LibGmsh

////////////////////////////////////////////////////////////////////////////////

} // gmsh
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_LibGmsh_hpp
