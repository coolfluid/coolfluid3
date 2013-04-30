// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Libphysics_hpp
#define cf3_Libphysics_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro physics_API
/// @note build system defines COOLFLUID_SOLVER_EXPORTS when compiling MeshTools files
#ifdef COOLFLUID_SOLVER_EXPORTS
#   define physics_API      CF3_EXPORT_API
#   define physics_TEMPLATE
#else
#   define physics_API      CF3_IMPORT_API
#   define physics_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

  /// Basic Classes for Physics applications used by CF
  namespace physics {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library Physics
  /// @author Tiago Quintino
  /// @author Martin Vymazal
  class physics_API LibPhysics : public common::Library {

  public:

    
    

    /// Constructor
    LibPhysics ( const std::string& name) : common::Library(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "cf3.physics"; }

    /// Static function that returns the library name.
    /// Must be implemented for Library registration
    /// @return name of the library
    static std::string library_name() { return "Physics"; }

    /// Static function that returns the description of the library.
    /// Must be implemented for Library registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the Physics API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibPhysics"; }
  }; // end LibPhysics

////////////////////////////////////////////////////////////////////////////////

} // physics
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Libphysics_hpp
