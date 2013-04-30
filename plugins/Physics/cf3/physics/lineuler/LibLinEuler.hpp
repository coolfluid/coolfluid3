// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_physics_LinEuler_LibLinEuler_hpp
#define cf3_physics_LinEuler_LibLinEuler_hpp


#include "cf3/common/Library.hpp"

/// Define the macro LinEuler_API
/// @note build system defines COOLFLUID_PHYSICS_LINEULER_EXPORTS when compiling LinEuler files
#ifdef COOLFLUID_PHYSICS_LINEULER_EXPORTS
#   define LinEuler_API      CF3_EXPORT_API
#   define TEMPLATE
#else
#   define LinEuler_API      CF3_IMPORT_API
#   define LinEuler_TEMPLATE CF3_TEMPLATE_EXTERN
#endif


namespace cf3 {
namespace physics {

/// @brief %Linearized Euler equations for sound propagation
///
/// LinEuler library
/// @author Tiago Quintino
namespace LinEuler {

////////////////////////////////////////////////////////////////////////////////////////////

/// Class defines the LinEuler library
/// @author Tiago Quintino
class LinEuler_API LibLinEuler : public common::Library
{
public:

  
  

  /// Constructor
  LibLinEuler ( const std::string& name) : common::Library(name) { }

  virtual ~LibLinEuler() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.physics.LinEuler"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "LinEuler"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements linearized Euler equations for acoustic propagation";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibLinEuler"; }

}; // end LibLinEuler

////////////////////////////////////////////////////////////////////////////////////////////

} // LinEuler
} // physics
} // cf3

#endif // cf3_physics_LinEuler_LibLinEuler_hpp

