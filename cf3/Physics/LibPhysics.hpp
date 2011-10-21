// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_LibPhysics_hpp
#define cf3_LibPhysics_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Physics_API
/// @note build system defines COOLFLUID_SOLVER_EXPORTS when compiling MeshTools files
#ifdef COOLFLUID_SOLVER_EXPORTS
#   define Physics_API      CF3_EXPORT_API
#   define Physics_TEMPLATE
#else
#   define Physics_API      CF3_IMPORT_API
#   define Physics_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

  /// Basic Classes for Physics applications used by CF
  namespace Physics {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library Physics
  /// @author Tiago Quintino
  /// @author Martin Vymazal
  class Physics_API LibPhysics : public common::Library {

  public:

    typedef boost::shared_ptr<LibPhysics> Ptr;
    typedef boost::shared_ptr<LibPhysics const> ConstPtr;

    /// Constructor
    LibPhysics ( const std::string& name) : common::Library(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.Physics"; }

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

  protected:

    /// initiate library
    virtual void initiate_impl();

    /// terminate library
    virtual void terminate_impl();

  }; // end LibPhysics

////////////////////////////////////////////////////////////////////////////////

} // Physics
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_LibPhysics_hpp
