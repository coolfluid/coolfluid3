// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_CriterionConvergence_hpp
#define cf3_solver_actions_CriterionConvergence_hpp

#include "solver/actions/LibActions.hpp"
#include "solver/actions/Criterion.hpp"
#include "solver/actions/Iterate.hpp"

#include "LibUFEM.hpp"
#include "LibUFEM.hpp"

#include "solver/Time.hpp"

namespace cf3 {
namespace UFEM {

////////////////////////////////////////////////////////////////////////////////////////////

class solver_actions_API CouplingIterationControl : public common::Action {

public: // typedefs

public: // functions

  /// Contructor
  /// @param name of the component
  CouplingIterationControl ( const std::string& name );

  /// Virtual destructor
  virtual ~CouplingIterationControl();

  /// Get the class name
  static std::string type_name () { return "CouplingIterationControl"; }

  /// Execute the control for the iteration loop
  virtual void execute();

private:

  /// component where to access the current iteration
  Handle<common::Component> m_pseudo_iter_comp;

  /// component for max number of iterations without iterating between two solvers to advance in time

  int m_interval;
  int m_int_time;
  std::vector<std::string> m_list_of_disabled_actions;
  Handle<solver::Time> m_time;

};

////////////////////////////////////////////////////////////////////////////////////////////
} // UFEM
} // cf3

#endif // cf3_solver_actions_CriterionConvergence_hpp
