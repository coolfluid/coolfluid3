// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_CCriterionMilestoneIteration_hpp
#define cf3_solver_actions_CCriterionMilestoneIteration_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/actions/LibActions.hpp"
#include "solver/actions/CCriterion.hpp"

namespace cf3 {
namespace solver {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

/// CCriterionMilestoneIteration models a Unsteady PDE problem
/// @author Willem Deconinck
class solver_actions_API CCriterionMilestoneIteration : public CCriterion {

public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  CCriterionMilestoneIteration ( const std::string& name );

  /// Virtual destructor
  virtual ~CCriterionMilestoneIteration();

  /// Get the class name
  static std::string type_name () { return "CCriterionMilestoneIteration"; }

  /// Simulates this model
  virtual bool operator()();

private:

  Handle<CTime> m_time;

};

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_CCriterionMilestoneIteration_hpp
