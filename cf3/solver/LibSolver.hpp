// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_LibSolver_hpp
#define cf3_solver_LibSolver_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro solver_API
/// @note build system defines COOLFLUID_SOLVER_EXPORTS when compiling MeshTools files
#ifdef COOLFLUID_SOLVER_EXPORTS
#   define solver_API      CF3_EXPORT_API
#   define solver_TEMPLATE
#else
#   define solver_API      CF3_IMPORT_API
#   define solver_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

  /// Basic Classes for Solver applications used by CF
  namespace solver {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library Solver
  /// @author Tiago Quintino
  /// @author Martin Vymazal
  class solver_API LibSolver :
      public common::Library
  {
  public:

    
    

    /// Constructor
    LibSolver ( const std::string& name) : common::Library(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "cf3.solver"; }

    /// Static function that returns the library name.
    /// Must be implemented for Library registration
    /// @return name of the library
    static std::string library_name() { return "Solver"; }

    /// Static function that returns the description of the library.
    /// Must be implemented for Library registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the Solver API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibSolver"; }

  }; // end LibSolver

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_LibSolver_hpp
