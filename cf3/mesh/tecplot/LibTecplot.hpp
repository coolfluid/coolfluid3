// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_LibTecplot_hpp
#define cf3_LibTecplot_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro tecplot_API
/// @note build system defines COOLFLUID_TECPLOT_EXPORTS when compiling tecplot files
#ifdef COOLFLUID_TECPLOT_EXPORTS
#   define tecplot_API      CF3_EXPORT_API
#   define tecplot_TEMPLATE
#else
#   define tecplot_API      CF3_IMPORT_API
#   define tecplot_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

/// @brief Library for I/O of the tecplot format
namespace tecplot {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the tecplot mesh format operations
/// @author Willem Deconinck
class tecplot_API LibTecplot :
    public common::Library {

public:

  
  

  /// Constructor
  LibTecplot ( const std::string& name) : common::Library(name) {   }

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.mesh.tecplot"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "tecplot"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the tecplot mesh format operations.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibTecplot"; }
}; // end LibTecplot

////////////////////////////////////////////////////////////////////////////////

} // tecplot
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_LibTecplot_hpp
