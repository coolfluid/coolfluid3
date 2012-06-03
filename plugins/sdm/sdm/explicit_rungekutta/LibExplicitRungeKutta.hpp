// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file sdm/explicit_rungekutta/LibExplicitRungeKutta.hpp
/// @author Willem Deconinck
///
/// This file defines the cf3::smd::explicit_rungekutta namespace
/// and the LibExplicitRungeKutta library

#ifndef cf3_sdm_LibExplicitRungeKutta_hpp
#define cf3_sdm_LibExplicitRungeKutta_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro sdm_explicit_rungekutta_API
/// @note build system defines COOLFLUID_SDM_EXPLICIT_RUNGEKUTTA_EXPORTS when compiling sdm files
#ifdef COOLFLUID_SDM_EXPLICIT_RUNGEKUTTA_EXPORTS
#   define sdm_explicit_rungekutta_API      CF3_EXPORT_API
#   define sdm_explicit_rungekutta_TEMPLATE
#else
#   define sdm_explicit_rungekutta_API      CF3_IMPORT_API
#   define sdm_explicit_rungekutta_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
/// @brief Explicit Runge-Kutta time-integration namespace
///
/// @author Willem Deconinck
namespace explicit_rungekutta {

////////////////////////////////////////////////////////////////////////////////

/// @brief Defines the Explicit Runge-Kutta time-integration library
///
/// @author Willem Deconinck
class sdm_explicit_rungekutta_API LibExplicitRungeKutta :
    public cf3::common::Library
{
public:

  /// Constructor
  LibExplicitRungeKutta ( const std::string& name) : cf3::common::Library(name) { }

  virtual ~LibExplicitRungeKutta() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.sdm.explicit_rungekutta"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "explicit_rungekutta"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the Explicit Runge-Kutta time-integration method";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibExplicitRungeKutta"; }
  
  virtual void initiate();
}; // end LibExplicitRungeKutta

////////////////////////////////////////////////////////////////////////////////

} // explicit_rungekutta
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_LibExplicitRungeKutta_hpp
