// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_hpp
#define CF_Mesh_SF_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"
#include "Common/ModuleRegister.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro SF_API
/// @note build system defines SF_EXPORTS when compiling SF files
#ifdef SF_EXPORTS
#   define SF_API      CF_EXPORT_API
#   define SF_TEMPLATE
#else
#   define SF_API      CF_IMPORT_API
#   define SF_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////
    
  /// Shape functions module
  /// @author Tiago Quintino, Willem Deconinck, Bart Janssens
  class SFLib :
      public Common::ModuleRegister<SFLib>
  {
  public:

    /// Static function that returns the module name.
    /// Must be implemented for the ModuleRegister template
    /// @return name of the module
    static std::string getModuleName() { return "SF"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for the ModuleRegister template
    /// @return descripton of the module
    static std::string getModuleDescription()
    {
      return "This library implements the shape functions.";
    }

    /// Gets the Class name
    static std::string type_name() { return "SFLib"; }

  }; // end SFLib

////////////////////////////////////////////////////////////////////////////////

} // namespace SF
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_SF_hpp
