// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_Conditional_hpp
#define cf3_solver_Conditional_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/actions/LibActions.hpp"
#include "common/Action.hpp"

namespace cf3 {
namespace solver {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

/// Action that gets executed Conditionalally
/// @author Tiago Quintino
class solver_actions_API Conditional : public common::Action {

public: // functions

  /// Contructor
  /// @param name of the component
  Conditional ( const std::string& name );

  /// Virtual destructor
  virtual ~Conditional();

  /// Get the class name
  static std::string type_name () { return "Conditional"; }

  /// executes child actions if the if criterion is met.
  /// If no criterion is added, default = true
  virtual void execute();

protected: // data

};

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_Conditional_hpp
