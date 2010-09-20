// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_OpenFOAM_OpenFOAMAPI_hpp
#define CF_Mesh_OpenFOAM_OpenFOAMAPI_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"
#include "Common/ModuleRegister.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro OpenFOAM_API
/// @note build system defines OpenFOAM_EXPORTS when compiling OpenFOAM files
#ifdef OpenFOAM_EXPORTS
#   define OpenFOAM_API      CF_EXPORT_API
#   define OpenFOAM_TEMPLATE
#else
#   define OpenFOAM_API      CF_IMPORT_API
#   define OpenFOAM_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace OpenFOAM {

////////////////////////////////////////////////////////////////////////////////
    
/// Class defines the OpenFOAMtral mesh format operations
/// @author Willem Deconinck
class OpenFOAMLib :
    public Common::ModuleRegister<OpenFOAMLib>
{
public:

  /// Static function that returns the module name.
  /// Must be implemented for the ModuleRegister template
  /// @return name of the module
  static std::string getModuleName() { return "OpenFOAM"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for the ModuleRegister template
  /// @return descripton of the module
  static std::string getModuleDescription()
  {
    return "This library implements some OpenFOAM compatibility functions.";
  }

  /// Gets the Class name
  static std::string type_name() { return "OpenFOAMLib"; }

}; // end OpenFOAMLib

////////////////////////////////////////////////////////////////////////////////

} // namespace OpenFOAM
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_OpenFOAM_OpenFOAMAPI_hpp
