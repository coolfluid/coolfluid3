// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_LibSmurf_hpp
#define cf3_LibSmurf_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro smurf_API
/// @note build system defines COOLFLUID_SMURF_EXPORTS when compiling smurf files
#ifdef COOLFLUID_SMURF_EXPORTS
#   define smurf_API      CF3_EXPORT_API
#   define smurf_TEMPLATE
#else
#   define smurf_API      CF3_IMPORT_API
#   define smurf_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

/// @brief Library for I/O of the smurf format
namespace smurf {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the smurf mesh format operations
/// @author Willem Deconinck
class smurf_API LibSmurf :
    public common::Library {

public:




  /// Constructor
  LibSmurf ( const std::string& name) : common::Library(name) {   }

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.mesh.smurf"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "smurf"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the smurf mesh format operations.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibSmurf"; }
}; // end LibSmurf

////////////////////////////////////////////////////////////////////////////////

} // smurf
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_LibSmurf_hpp
