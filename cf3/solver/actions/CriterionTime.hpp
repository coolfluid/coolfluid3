// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_CriterionTime_hpp
#define cf3_solver_actions_CriterionTime_hpp

#include "solver/actions/LibActions.hpp"
#include "solver/actions/Criterion.hpp"

namespace cf3 {
namespace solver {
namespace actions {

////////////////////////////////////////////////////////////////////////////////////////////

/// Criterion for time limit
/// @author Willem Deconinck
/// @author Tiago Quintino
class solver_actions_API CriterionTime : public Criterion
{
public: // functions

  /// Contructor
  /// @param name of the component
  CriterionTime ( const std::string& name );

  /// Virtual destructor
  virtual ~CriterionTime();

  /// Get the class name
  static std::string type_name () { return "CriterionTime"; }

  /// Simulates this model
  virtual bool operator()();

private:

  Handle<Time> m_time;

  Real m_tolerance;

};

////////////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

#endif // cf3_solver_actions_CriterionTime_hpp
