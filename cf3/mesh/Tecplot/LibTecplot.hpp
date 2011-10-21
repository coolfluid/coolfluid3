// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_LibTecplot_hpp
#define cf3_LibTecplot_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Tecplot_API
/// @note build system defines COOLFLUID_GMSH_EXPORTS when compiling Tecplot files
#ifdef COOLFLUID_GMSH_EXPORTS
#   define Tecplot_API      CF3_EXPORT_API
#   define Tecplot_TEMPLATE
#else
#   define Tecplot_API      CF3_IMPORT_API
#   define Tecplot_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
  
/// @brief Library for I/O of the Tecplot format 
namespace Tecplot {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the Tecplot mesh format operations
/// @author Willem Deconinck
class Tecplot_API LibTecplot :
    public common::Library {

public:

  typedef boost::shared_ptr<LibTecplot> Ptr;
  typedef boost::shared_ptr<LibTecplot const> ConstPtr;

  /// Constructor
  LibTecplot ( const std::string& name) : common::Library(name) {   }

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Mesh.Tecplot"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "Tecplot"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the Tecplot mesh format operations.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibTecplot"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibTecplot

////////////////////////////////////////////////////////////////////////////////

} // Tecplot
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_LibTecplot_hpp
