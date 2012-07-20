// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file sdm/lusgs/LibLUSGS.hpp
/// @author Willem Deconinck
///
/// This file defines the cf3::smd::lusgs namespace
/// and the LibLUSGS library

#ifndef cf3_sdm_LibLUSGS_hpp
#define cf3_sdm_LibLUSGS_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro sdm_lusgs_API
/// @note build system defines COOLFLUID_SDM_lusgs_EXPORTS when compiling sdm files
#ifdef COOLFLUID_SDM_lusgs_EXPORTS
#   define sdm_lusgs_API      CF3_EXPORT_API
#   define sdm_lusgs_TEMPLATE
#else
#   define sdm_lusgs_API      CF3_IMPORT_API
#   define sdm_lusgs_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
/// @brief lusgs namespace
///
/// @author Willem Deconinck
namespace lusgs {

////////////////////////////////////////////////////////////////////////////////

/// @brief Defines the LUSGS library
///
/// @author Willem Deconinck
class sdm_lusgs_API LibLUSGS :
    public cf3::common::Library
{
public:

  /// Constructor
  LibLUSGS ( const std::string& name) : cf3::common::Library(name) { }

  virtual ~LibLUSGS() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.sdm.lusgs"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "lusgs"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the LUSGS method";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibLUSGS"; }
  
  virtual void initiate();
}; // end LibLUSGS

////////////////////////////////////////////////////////////////////////////////

} // lusgs
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_LibLUSGS_hpp
