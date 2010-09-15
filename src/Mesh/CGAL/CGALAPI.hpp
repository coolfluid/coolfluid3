// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_CGALAPI_hpp
#define CF_CGALAPI_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ModuleRegister.hpp"
#include "Common/ExportAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro CGAL_API
/// @note build system defines CGAL_EXPORTS when compiling CGAL files
#ifdef CGAL_EXPORTS
#   define CGAL_API      CF_EXPORT_API
#   define CGAL_TEMPLATE
#else
#   define CGAL_API      CF_IMPORT_API
#   define CGAL_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace CGAL {

////////////////////////////////////////////////////////////////////////////////
    
/// Class defines the CGAL mesh format operations
/// @author Bart Janssens
class CGAL_API CGALLib :
    public CF::Common::ModuleRegister<CGALLib>
{
public:

  /// Static function that returns the module name.
  /// Must be implemented for the ModuleRegister template
  /// @return name of the module
  static std::string getModuleName() { return "CGAL"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for the ModuleRegister template
  /// @return descripton of the module
  static std::string getModuleDescription()
  {
    return "This library provides an interface for the CGAL 3D tetrahedral mesher.";
  }

  /// Gets the Class name
  static std::string getClassName() { return "CGALLib"; }

}; // end CGALLib

////////////////////////////////////////////////////////////////////////////////

} // namespace CGAL
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_CGAL_hpp
