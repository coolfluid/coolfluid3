// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file sdm/implicit/LibImplicit.hpp
/// @author Willem Deconinck
///
/// This file defines the cf3::smd::lusgs namespace
/// and the LibImplicit library

#ifndef cf3_sdm_LibImplicit_hpp
#define cf3_sdm_LibImplicit_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro sdm_implicit_API
/// @note build system defines COOLFLUID_SDM_lusgs_EXPORTS when compiling sdm files
#ifdef COOLFLUID_SDM_IMPLICIT_EXPORTS
#   define sdm_implicit_API      CF3_EXPORT_API
#   define sdm_implicit_TEMPLATE
#else
#   define sdm_implicit_API      CF3_IMPORT_API
#   define sdm_implicit_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
/// @brief implicit namespace
///
/// @author Willem Deconinck
namespace implicit {

////////////////////////////////////////////////////////////////////////////////

/// @brief Defines the implicit time-integration library
///
/// @author Willem Deconinck
class sdm_implicit_API LibImplicit :
    public cf3::common::Library
{
public:

  /// Constructor
  LibImplicit ( const std::string& name) : cf3::common::Library(name) { }

  virtual ~LibImplicit() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.sdm.implicit"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "implicit"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements implicit time-integration schemes";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibImplicit"; }
  
  virtual void initiate();
}; // end LibImplicit

////////////////////////////////////////////////////////////////////////////////

} // implicit
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_LibImplicit_hpp
