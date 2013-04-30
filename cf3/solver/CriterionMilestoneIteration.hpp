// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_CriterionMilestoneIteration_hpp
#define cf3_solver_CriterionMilestoneIteration_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/Criterion.hpp"

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////

/// CriterionMilestoneIteration models a Unsteady PDE problem
/// @author Willem Deconinck
class solver_API CriterionMilestoneIteration : public Criterion {

public: // functions

  /// Contructor
  /// @param name of the component
  CriterionMilestoneIteration ( const std::string& name );

  /// Virtual destructor
  virtual ~CriterionMilestoneIteration();

  /// Get the class name
  static std::string type_name () { return "CriterionMilestoneIteration"; }

  /// Simulates this model
  virtual bool operator()();

private:

  Handle<Time> m_time;

};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_CriterionMilestoneIteration_hpp
