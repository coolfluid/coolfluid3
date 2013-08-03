// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_ReadRestartFile_hpp
#define cf3_solver_actions_ReadRestartFile_hpp

#include "common/Action.hpp"
#include "solver/actions/LibActions.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

/// Read out a restartfile, designed to be loaded into an already-created mesh
class solver_actions_API ReadRestartFile : public common::Action
{
public: // functions
  /// Contructor
  /// @param name of the component
  ReadRestartFile ( const std::string& name );

  /// Virtual destructor
  virtual ~ReadRestartFile() {}

  /// Get the class name
  static std::string type_name () { return "ReadRestartFile"; }

  /// execute the action
  virtual void execute ();
};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_ReadRestartFile_hpp
