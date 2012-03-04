// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_navierstokes_LibNavierStokes_hpp
#define cf3_sdm_navierstokes_LibNavierStokes_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro navierstokes_API
/// @note build system defines COOLFLUID_sdm_NAVIERSTOKES_EXPORTS when compiling navierstokes files
#ifdef COOLFLUID_sdm_NAVIERSTOKES_EXPORTS
#   define sdm_navierstokes_API      CF3_EXPORT_API
#   define sdm_navierstokes_TEMPLATE
#else
#   define sdm_navierstokes_API      CF3_IMPORT_API
#   define sdm_navierstokes_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace navierstokesmovingreference {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the navierstokes library
class sdm_navierstokes_API LibNavierStokesMovingReference : public common::Library
{
public:

  
  

  /// Constructor
  LibNavierStokesMovingReference ( const std::string& name) : common::Library(name) { }

  virtual ~LibNavierStokesMovingReference() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.sdm.navierstokesmovingreference"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "navierstokesmovingreference"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements navierstokes equations";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibNavierStokes"; }

}; // end LibNavierStokesMovingReference

////////////////////////////////////////////////////////////////////////////////

} // navierstokesmovingreference
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_navierstokes_LibNavierStokesMovingReference_hpp

