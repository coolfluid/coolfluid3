// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_AdvanceTime_hpp
#define cf3_solver_AdvanceTime_hpp

#include "solver/actions/LibActions.hpp"
#include "solver/Action.hpp"

namespace cf3 {
namespace solver {
  class Time;
namespace actions {

////////////////////////////////////////////////////////////////////////////////

/// AdvanceTime
/// @author Willem Deconinck
class solver_actions_API AdvanceTime : public solver::Action
{
public: // functions

  /// Contructor
  /// @param name of the component
  AdvanceTime ( const std::string& name );

  /// Virtual destructor
  virtual ~AdvanceTime();

  /// Get the class name
  static std::string type_name () { return "AdvanceTime"; }

  /// Simulates this model
  virtual void execute();

  /// @returns the time component
  solver::Time& time();

private:

  Handle< solver::Time > m_time; ///< time used by this action

};

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

#endif // cf3_solver_AdvanceTime_hpp
