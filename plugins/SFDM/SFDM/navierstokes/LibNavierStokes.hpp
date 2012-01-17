// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_navierstokes_LibNavierStokes_hpp
#define cf3_SFDM_navierstokes_LibNavierStokes_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro navierstokes_API
/// @note build system defines COOLFLUID_SFDM_NAVIERSTOKES_EXPORTS when compiling navierstokes files
#ifdef COOLFLUID_SFDM_NAVIERSTOKES_EXPORTS
#   define SFDM_navierstokes_API      CF3_EXPORT_API
#   define SFDM_navierstokes_TEMPLATE
#else
#   define SFDM_navierstokes_API      CF3_IMPORT_API
#   define SFDM_navierstokes_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {
namespace navierstokes {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the navierstokes library
class SFDM_navierstokes_API LibNavierStokes : public common::Library
{
public:

  
  

  /// Constructor
  LibNavierStokes ( const std::string& name) : common::Library(name) { }

  virtual ~LibNavierStokes() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.SFDM.navierstokes"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "navierstokes"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements navierstokes equations";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibNavierStokes"; }

}; // end LibNavierStokes

////////////////////////////////////////////////////////////////////////////////

} // navierstokes
} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_navierstokes_LibNavierStokes_hpp

