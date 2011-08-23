// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_LibVTKXML_hpp
#define CF_LibVTKXML_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro VTKXML_API
/// @note build system defines COOLFLUID_GMSH_EXPORTS when compiling VTKXML files
#ifdef COOLFLUID_GMSH_EXPORTS
#   define VTKXML_API      CF_EXPORT_API
#   define VTKXML_TEMPLATE
#else
#   define VTKXML_API      CF_IMPORT_API
#   define VTKXML_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

/// @brief Library for I/O of the VTK XML format
namespace VTKXML {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the VTKXMLtral mesh format operations
/// @author Willem Deconinck
class VTKXML_API LibVTKXML :
    public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibVTKXML> Ptr;
  typedef boost::shared_ptr<LibVTKXML const> ConstPtr;

  /// Constructor
  LibVTKXML ( const std::string& name) : Common::CLibrary(name) {   }

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Mesh.VTKXML"; }

  /// Static function that returns the library name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "VTKXML"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the VTKXML mesh format operations.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibVTKXML"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibVTKXML

////////////////////////////////////////////////////////////////////////////////

} // VTKXML
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_LibVTKXML_hpp
