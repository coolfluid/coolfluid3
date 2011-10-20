// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Solver_Actions_LibActions_hpp
#define cf3_Solver_Actions_LibActions_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Actions_API
/// @note build system defines COOLFLUID_SOLVER_ACTIONS_EXPORTS when compiling files
#ifdef COOLFLUID_SOLVER_ACTIONS_EXPORTS
#   define Solver_Actions_API      CF3_EXPORT_API
#   define Solver_Actions_TEMPLATE
#else
#   define Solver_Actions_API      CF3_IMPORT_API
#   define Solver_Actions_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Solver {
namespace Actions {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the initialization and termination of the library Actions
class Solver_Actions_API LibActions :
    public common::CLibrary
{
public:

  typedef boost::shared_ptr<LibActions> Ptr;
  typedef boost::shared_ptr<LibActions const> ConstPtr;

  /// Constructor
  LibActions ( const std::string& name) : common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Solver.Actions"; }


  /// Static function that returns the library name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "Actions"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the Actions API.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibActions"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibActions

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Solver_Actions_LibActions_hpp
