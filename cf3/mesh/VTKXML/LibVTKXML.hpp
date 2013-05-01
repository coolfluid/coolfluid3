// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_LibVTKXML_hpp
#define cf3_LibVTKXML_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro VTKXML_API
/// @note build system defines COOLFLUID_GMSH_EXPORTS when compiling VTKXML files
#ifdef COOLFLUID_GMSH_EXPORTS
#   define VTKXML_API      CF3_EXPORT_API
#   define VTKXML_TEMPLATE
#else
#   define VTKXML_API      CF3_IMPORT_API
#   define VTKXML_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

/// @brief Library for I/O of the VTK XML format
namespace VTKXML {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the VTKXMLtral mesh format operations
/// @author Willem Deconinck
class VTKXML_API LibVTKXML :
    public common::Library
{
public:

  
  

  /// Constructor
  LibVTKXML ( const std::string& name) : common::Library(name) {   }

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.mesh.VTKXML"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "VTKXML"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the VTKXML mesh format operations.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibVTKXML"; }
}; // end LibVTKXML

////////////////////////////////////////////////////////////////////////////////

} // VTKXML
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_LibVTKXML_hpp
