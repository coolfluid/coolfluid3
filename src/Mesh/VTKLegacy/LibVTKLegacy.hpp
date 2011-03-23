// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_LibVTKLegacy_hpp
#define CF_LibVTKLegacy_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro VTKLegacy_API
/// @note build system defines COOLFLUID_GMSH_EXPORTS when compiling VTKLegacy files
#ifdef COOLFLUID_GMSH_EXPORTS
#   define VTKLegacy_API      CF_EXPORT_API
#   define VTKLegacy_TEMPLATE
#else
#   define VTKLegacy_API      CF_IMPORT_API
#   define VTKLegacy_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace VTKLegacy {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the VTKLegacytral mesh format operations
/// @author Willem Deconinck
class VTKLegacy_API LibVTKLegacy :
    public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibVTKLegacy> Ptr;
  typedef boost::shared_ptr<LibVTKLegacy const> ConstPtr;

  /// Constructor
  LibVTKLegacy ( const std::string& name) : Common::CLibrary(name) {   }

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Mesh.VTKLegacy"; }

  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "VTKLegacy"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the VTKLegacy mesh format operations.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibVTKLegacy"; }

  /// initiate library
  virtual void initiate();

  /// terminate library
  virtual void terminate();

}; // end LibVTKLegacy

////////////////////////////////////////////////////////////////////////////////

} // VTKLegacy
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_LibVTKLegacy_hpp
