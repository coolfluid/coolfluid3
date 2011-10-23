// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_Actions_CCriterionMilestoneIteration_hpp
#define cf3_solver_Actions_CCriterionMilestoneIteration_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/Actions/LibActions.hpp"
#include "solver/Actions/CCriterion.hpp"

namespace cf3 {
namespace solver {
namespace Actions {

////////////////////////////////////////////////////////////////////////////////

/// CCriterionMilestoneIteration models a Unsteady PDE problem
/// @author Willem Deconinck
class solver_Actions_API CCriterionMilestoneIteration : public CCriterion {

public: // typedefs

  typedef boost::shared_ptr<CCriterionMilestoneIteration> Ptr;
  typedef boost::shared_ptr<CCriterionMilestoneIteration const> ConstPtr;

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

  boost::weak_ptr<CTime> m_time;

};

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_Actions_CCriterionMilestoneIteration_hpp
