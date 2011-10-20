// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_LibSolver_hpp
#define cf3_LibSolver_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Solver_API
/// @note build system defines COOLFLUID_SOLVER_EXPORTS when compiling MeshTools files
#ifdef COOLFLUID_SOLVER_EXPORTS
#   define Solver_API      CF3_EXPORT_API
#   define Solver_TEMPLATE
#else
#   define Solver_API      CF3_IMPORT_API
#   define Solver_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

  /// Basic Classes for Solver applications used by CF
  namespace Solver {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library Solver
  /// @author Tiago Quintino
  /// @author Martin Vymazal
  class Solver_API LibSolver :
      public common::CLibrary
  {
  public:

    typedef boost::shared_ptr<LibSolver> Ptr;
    typedef boost::shared_ptr<LibSolver const> ConstPtr;

    /// Constructor
    LibSolver ( const std::string& name) : common::CLibrary(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.Solver"; }

    /// Static function that returns the library name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "Solver"; }

    /// Static function that returns the description of the library.
    /// Must be implemented for CLibrary registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the Solver API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibSolver"; }

  protected:

    /// initiate library
    virtual void initiate_impl();

    /// terminate library
    virtual void terminate_impl();

  }; // end LibSolver

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_LibSolver_hpp
