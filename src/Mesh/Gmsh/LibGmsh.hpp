// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_LibGmsh_hpp
#define CF_LibGmsh_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/LibraryRegister.hpp"
#include "Common/ExportAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Gmsh_API
/// @note build system defines COOLFLUID_GMSH_EXPORTS when compiling Gmsh files
#ifdef COOLFLUID_GMSH_EXPORTS
#   define Gmsh_API      CF_EXPORT_API
#   define Gmsh_TEMPLATE
#else
#   define Gmsh_API      CF_IMPORT_API
#   define Gmsh_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Gmsh {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the Gmshtral mesh format operations
/// @author Willem Deconinck
class Gmsh_API LibGmsh :
    public Common::LibraryRegister<LibGmsh>
{
public:

  /// Static function that returns the module name.
  /// Must be implemented for the LibraryRegister template
  /// @return name of the library
  static std::string library_name() { return "Gmsh"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for the LibraryRegister template
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the Gmsh mesh format operations.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibGmsh"; }

  /// initiate library
  virtual void initiate();

  /// terminate library
  virtual void terminate();

}; // end LibGmsh

////////////////////////////////////////////////////////////////////////////////

} // Gmsh
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_LibGmsh_hpp
