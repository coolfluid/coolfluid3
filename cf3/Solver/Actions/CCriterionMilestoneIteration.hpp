// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Solver_Actions_CCriterionMilestoneIteration_hpp
#define cf3_Solver_Actions_CCriterionMilestoneIteration_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/Actions/LibActions.hpp"
#include "Solver/Actions/CCriterion.hpp"

namespace cf3 {
namespace Solver {
namespace Actions {

////////////////////////////////////////////////////////////////////////////////

/// CCriterionMilestoneIteration models a Unsteady PDE problem
/// @author Willem Deconinck
class Solver_Actions_API CCriterionMilestoneIteration : public CCriterion {

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
} // Solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Solver_Actions_CCriterionMilestoneIteration_hpp
