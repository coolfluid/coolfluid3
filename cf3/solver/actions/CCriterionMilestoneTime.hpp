// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_CCriterionMilestoneTime_hpp
#define cf3_solver_actions_CCriterionMilestoneTime_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/actions/LibActions.hpp"
#include "solver/actions/CCriterion.hpp"

namespace cf3 {
namespace solver {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

/// CCriterionMilestoneTime
/// @author Willem Deconinck
class solver_actions_API CCriterionMilestoneTime : public CCriterion {

public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  CCriterionMilestoneTime ( const std::string& name );

  /// Virtual destructor
  virtual ~CCriterionMilestoneTime();

  /// Get the class name
  static std::string type_name () { return "CCriterionMilestoneTime"; }

  /// Simulates this model
  virtual bool operator()();

private:

  Handle<CTime> m_time;

  Real m_tolerance;

};

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_CCriterionMilestoneTime_hpp
