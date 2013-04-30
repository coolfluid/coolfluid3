// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_LibActions_hpp
#define cf3_solver_actions_LibActions_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro actions_API
/// @note build system defines COOLFLUID_SOLVER_ACTIONS_EXPORTS when compiling files
#ifdef COOLFLUID_SOLVER_ACTIONS_EXPORTS
#   define solver_actions_API      CF3_EXPORT_API
#   define solver_actions_TEMPLATE
#else
#   define solver_actions_API      CF3_IMPORT_API
#   define solver_actions_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the initialization and termination of the library actions
class solver_actions_API LibActions :
    public common::Library
{
public:

  
  

  /// Constructor
  LibActions ( const std::string& name) : common::Library(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.solver.actions"; }


  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "actions"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the actions API.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibActions"; }

}; // end LibActions

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_LibActions_hpp
