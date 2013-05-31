// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_LibVTK_hpp
#define CF_LibVTK_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro VTK_API
/// @note build system defines COOLFLUID_VTK_EXPORTS when compiling VTK files
#ifdef COOLFLUID_VTK_EXPORTS
#   define VTK_API      CF3_EXPORT_API
#   define VTK_TEMPLATE
#else
#   define VTK_API      CF3_IMPORT_API
#   define VTK_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
  
/// @brief Classes for VTK operations
namespace vtk {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the VTK operations
/// @author Bart Janssens
class VTK_API LibVTK : public cf3::common::Library
{
public:

  
  

  /// Constructor
  LibVTK ( const std::string& name) : common::Library(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.vtk"; }

  /// Static function that returns the module name.
  /// Must be implemented for the LibraryRegister template
  /// @return name of the module
  static std::string library_name() { return "vtk"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for the LibraryRegister template
  /// @return descripton of the module
  static std::string library_description()
  {
    return "This library provides an interface for mesh manipulation using VTK.";
  }

  /// Gets the Class name
  static std::string getClassName() { return "LibVTK"; }

}; // end VTKLib

////////////////////////////////////////////////////////////////////////////////

} // namespace vtk
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF_LibVTK_hpp
