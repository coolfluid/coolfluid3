// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_LibSolver_hpp
#define CF_LibSolver_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"
#include "Common/LibraryRegister.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Solver_API
/// @note build system defines COOLFLUID_SOLVER_EXPORTS when compiling MeshTools files
#ifdef COOLFLUID_SOLVER_EXPORTS
#   define Solver_API      CF_EXPORT_API
#   define Solver_TEMPLATE
#else
#   define Solver_API      CF_IMPORT_API
#   define Solver_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  /// Basic Classes for Solver applications used by CF
  namespace Solver {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library Solver
  /// @author Tiago Quintino
  /// @author Martin Vymazal
  class Solver_API LibSolver :
      public Common::LibraryRegister<LibSolver>
  {
  public:

    /// Static function that returns the module name.
    /// Must be implemented for the LibraryRegister template
    /// @return name of the module
    static std::string library_name() { return "Solver"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for the LibraryRegister template
    /// @return descripton of the module
    static std::string library_description()
    {
      return "This library implements the Solver API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibSolver"; }

    /// Start profiling
    virtual void initiate();

    /// Stop profiling
    virtual void terminate();

  }; // end LibSolver

////////////////////////////////////////////////////////////////////////////////

} // namespace Solver
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_LibSolver_hpp
