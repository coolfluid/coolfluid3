// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_LibCF3Mesh_hpp
#define cf3_LibCF3Mesh_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro CF3Mesh_API
/// @note build system defines COOLFLUID_GMSH_EXPORTS when compiling CF3Mesh files
#ifdef COOLFLUID_GMSH_EXPORTS
#   define CF3Mesh_API      CF3_EXPORT_API
#   define CF3Mesh_TEMPLATE
#else
#   define CF3Mesh_API      CF3_IMPORT_API
#   define CF3Mesh_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
  
/// @brief Library for I/O in the native CF3 mesh format
namespace cf3mesh {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the CF3Mesh mesh format operations
/// @author Bart Janssens
class CF3Mesh_API LibCF3Mesh :
    public common::Library
{
public:

  /// Constructor
  LibCF3Mesh ( const std::string& name) : common::Library(name) {   }

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.mesh.cf3mesh"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "cf3mesh"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the CF3Mesh mesh format operations.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibCF3Mesh"; }

}; // end LibCF3Mesh

////////////////////////////////////////////////////////////////////////////////

} // cf3mesh
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_LibCF3Mesh_hpp
