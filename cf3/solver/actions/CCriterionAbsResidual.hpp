// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_CCriterionAbsResidual_hpp
#define cf3_solver_actions_CCriterionAbsResidual_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/actions/LibActions.hpp"
#include "solver/actions/CCriterion.hpp"

namespace cf3 {
namespace solver {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

/// CCriterionAbsResidual models a Unsteady PDE problem
/// @author Tiago Quintino
class solver_actions_API CCriterionAbsResidual : public CCriterion {

public: // typedefs

  typedef boost::shared_ptr<CCriterionAbsResidual> Ptr;
  typedef boost::shared_ptr<CCriterionAbsResidual const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CCriterionAbsResidual ( const std::string& name );

  /// Virtual destructor
  virtual ~CCriterionAbsResidual();

  /// Get the class name
  static std::string type_name () { return "CCriterionAbsResidual"; }

  /// Simulates this model
  virtual bool operator()();
  
private:
  
  /// component where to access the current iteration
  boost::weak_ptr<Component> m_iter_comp;
  /// maximum number of iterations
  Uint m_max_iter;

};

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_CCriterionAbsResidual_hpp
