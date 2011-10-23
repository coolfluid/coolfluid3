// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_Actions_CCriterionMilestoneTime_hpp
#define cf3_solver_Actions_CCriterionMilestoneTime_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/Actions/LibActions.hpp"
#include "solver/Actions/CCriterion.hpp"

namespace cf3 {
namespace solver {
namespace Actions {

////////////////////////////////////////////////////////////////////////////////

/// CCriterionMilestoneTime
/// @author Willem Deconinck
class solver_Actions_API CCriterionMilestoneTime : public CCriterion {

public: // typedefs

  typedef boost::shared_ptr<CCriterionMilestoneTime> Ptr;
  typedef boost::shared_ptr<CCriterionMilestoneTime const> ConstPtr;

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

  boost::weak_ptr<CTime> m_time;

  Real m_tolerance;

};

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_Actions_CCriterionMilestoneTime_hpp
