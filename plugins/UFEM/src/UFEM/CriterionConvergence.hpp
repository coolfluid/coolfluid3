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

namespace cf3 {
namespace UFEM {

////////////////////////////////////////////////////////////////////////////////////////////

class solver_actions_API CriterionConvergence : public solver::actions::Criterion {

public: // typedefs




public: // functions

  /// Contructor
  /// @param name of the component
  CriterionConvergence ( const std::string& name );

  /// Virtual destructor
  virtual ~CriterionConvergence();

  /// Get the class name
  static std::string type_name () { return "CriterionConvergence"; }

  /// Simulates this model
  virtual bool operator()();

private:

  /// component where to access the current iteration
  Handle<common::Component> m_iter_comp;

  /// component for max number of iterations
  int m_max_iteration;

  Real m_min_error;
  Real m_max_error;
  Real m_cond_temperature;
  Real m_fluid_temperature;


};

////////////////////////////////////////////////////////////////////////////////////////////
} // UFEM
} // cf3

#endif // cf3_solver_actions_CriterionConvergence_hpp
