// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Tools_MeshDiff_LibMeshDiff_hpp
#define CF_Tools_MeshDiff_LibMeshDiff_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"
#include "Common/LibraryRegister.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro MeshDiff_API
/// @note build system defines COOLFLUID_MESHDIFF_EXPORTS when compiling
/// MeshDiff files
#ifdef COOLFLUID_MESHDIFF_EXPORTS
#   define MeshDiff_API      CF_EXPORT_API
#   define MeshDiff_TEMPLATE
#else
#   define MeshDiff_API      CF_IMPORT_API
#   define MeshDiff_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace MeshDiff {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library MeshDiff
  /// @author Tiago Quintino
  class MeshDiff_API LibMeshDiff :
      public Common::LibraryRegister<LibMeshDiff>
  {
  public:

    /// Static function that returns the module name.
    /// Must be implemented for the LibraryRegister template
    /// @return name of the module
    static std::string library_name() { return "MeshDiff"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for the LibraryRegister template
    /// @return descripton of the module
    static std::string library_description()
    {
      return "This library implements the MeshDiff manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibMeshDiff"; }

    /// initiate library
    virtual void initiate();

    /// terminate library
    virtual void terminate();

  }; // end LibMeshDiff

////////////////////////////////////////////////////////////////////////////////

} // MeshDiff
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Tools_MeshDiff_LibMeshDiff_hpp
