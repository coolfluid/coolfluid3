// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_physics_Scalar_LibScalar_hpp
#define cf3_physics_Scalar_LibScalar_hpp

////////////////////////////////////////////////////////////////////////////////

#include "cf3/common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Scalar_API
/// @note build system defines COOLFLUID_PHYSICS_SCALAR_EXPORTS when compiling Scalar files
#ifdef COOLFLUID_PHYSICS_SCALAR_EXPORTS
#   define Scalar_API      CF3_EXPORT_API
#   define Scalar_TEMPLATE
#else
#   define Scalar_API      CF3_IMPORT_API
#   define Scalar_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace physics {

/// @brief %Scalar transport equations
///
/// @author Tiago Quintino
namespace Scalar {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the Scalar library
/// @author Tiago Quintino
class Scalar_API LibScalar : public common::Library
{
public:

  
  

  /// Constructor
  LibScalar ( const std::string& name) : common::Library(name) { }

  virtual ~LibScalar() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.physics.Scalar"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "Scalar"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements Scalar equations";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibScalar"; }

}; // end LibScalar

////////////////////////////////////////////////////////////////////////////////

} // Scalar
} // physics
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_physics_Scalar_LibScalar_hpp

