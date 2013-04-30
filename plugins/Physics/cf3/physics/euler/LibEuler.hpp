// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_physics_euler_LibEuler_hpp
#define cf3_physics_euler_LibEuler_hpp

////////////////////////////////////////////////////////////////////////////////

#include "cf3/common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Euler_API
/// @note build system defines COOLFLUID_PHYSICS_NAVIERSTOKES_EXPORTS when compiling Advection diffusion files
#ifdef COOLFLUID_PHYSICS_NAVIERSTOKES_EXPORTS
#   define euler_API      CF3_EXPORT_API
#   define TEMPLATE
#else
#   define euler_API      CF3_IMPORT_API
#   define euler_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace physics {

/// @brief %Physics %Euler classes
///
/// Euler functionality for the %Physics is added in this library
/// @author Willem Deconinck
namespace euler {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the Euler library
/// @author Tiago Quintino
class euler_API LibEuler : public common::Library
{
public:

  
  

  /// Constructor
  LibEuler ( const std::string& name) : common::Library(name) { }

  virtual ~LibEuler() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.physics.euler"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "Euler"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements physics components for the Navier-Stokes fluid flow equations.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibEuler"; }

}; // end LibEuler

////////////////////////////////////////////////////////////////////////////////

} // euler
} // physics
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_physics_euler_LibEuler_hpp
