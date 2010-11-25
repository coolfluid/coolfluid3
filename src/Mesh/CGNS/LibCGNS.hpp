// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_LibCGNS_hpp
#define CF_LibCGNS_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/LibraryRegister.hpp"
#include "Common/ExportAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro CGNS_API
/// @note build system defines COOLFLUID_CGNS3_EXPORTS when compiling CGNS files
#ifdef COOLFLUID_CGNS3_EXPORTS
#   define CGNS_API      CF_EXPORT_API
#   define CGNS_TEMPLATE
#else
#   define CGNS_API      CF_IMPORT_API
#   define CGNS_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace CGNS {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the CGNS mesh format operations
/// @author Willem Deconinck
class CGNS_API LibCGNS :
    public CF::Common::LibraryRegister<LibCGNS>
{
public:

  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "CGNS"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the CGNS mesh format operations.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibCGNS"; }

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Mesh.CGNS"; }

  /// initiate library
  virtual void initiate();

  /// terminate library
  virtual void terminate();

}; // end LibCGNS

////////////////////////////////////////////////////////////////////////////////

} // CGNS
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_CGNS_hpp
