// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_CriterionAbsResidual_hpp
#define cf3_solver_CriterionAbsResidual_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/Criterion.hpp"

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////

/// CriterionAbsResidual models a Unsteady PDE problem
/// @author Tiago Quintino
class solver_API CriterionAbsResidual : public Criterion {

public: // functions

  /// Contructor
  /// @param name of the component
  CriterionAbsResidual ( const std::string& name );

  /// Virtual destructor
  virtual ~CriterionAbsResidual();

  /// Get the class name
  static std::string type_name () { return "CriterionAbsResidual"; }

  /// Simulates this model
  virtual bool operator()();
  
private:
  
  /// component where to access the current iteration
  Handle<Component> m_iter_comp;
  /// maximum number of iterations
  Uint m_max_iter;

};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_CriterionAbsResidual_hpp
