// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_LibCGNS_hpp
#define cf3_LibCGNS_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro CGNS_API
/// @note build system defines COOLFLUID_CGNS_EXPORTS when compiling CGNS files
#ifdef COOLFLUID_MESH_CGNS_EXPORTS
#   define Mesh_CGNS_API      CF3_EXPORT_API
#   define Mesh_CGNS_TEMPLATE
#else
#   define Mesh_CGNS_API      CF3_IMPORT_API
#   define Mesh_CGNS_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
  
/// @brief Library for I/O of the CGNS format
namespace CGNS {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the CGNS mesh format operations
/// @author Willem Deconinck
class Mesh_CGNS_API LibCGNS : public cf3::common::Library
{
public:

  
  

  /// Constructor
  LibCGNS ( const std::string& name) : common::Library(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.mesh.CGNS"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "CGNS"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the CGNS mesh format operations.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibCGNS"; }
}; // end LibCGNS

////////////////////////////////////////////////////////////////////////////////

} // CGNS
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_CGNS_hpp
