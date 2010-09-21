// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_MeshDiffAPI_hpp
#define CF_MeshDiffAPI_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"
#include "Common/ModuleRegister.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro MeshDiff_API
/// @note build system defines MeshDiff_EXPORTS when compiling MeshDiffTools files
#ifdef MeshDiff_EXPORTS
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

  /// Class defines the initialization and termination of the library MeshDiffDiff
  /// @author Tiago Quintino
  class MeshDiffLib :
      public Common::ModuleRegister<MeshDiffLib>
  {
  public:

    /// Static function that returns the module name.
    /// Must be implemented for the ModuleRegister template
    /// @return name of the module
    static std::string getModuleName() { return "MeshDiff"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for the ModuleRegister template
    /// @return descripton of the module
    static std::string getModuleDescription()
    {
      return "This library implements the MeshDiff manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "MeshDiffLib"; }

    /// Start profiling
    virtual void initiate();

    /// Stop profiling
    virtual void terminate();

  }; // end MeshDiffLib

////////////////////////////////////////////////////////////////////////////////

} // namespace MeshDiff
} // namespace Tools
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_MeshDiff_hpp
