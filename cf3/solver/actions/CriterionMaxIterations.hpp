// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_CriterionMaxIterations_hpp
#define cf3_solver_actions_CriterionMaxIterations_hpp

#include "solver/actions/LibActions.hpp"
#include "solver/actions/Criterion.hpp"

namespace cf3 {
namespace solver {
namespace actions {

////////////////////////////////////////////////////////////////////////////////////////////

/// Criterion for maximum number of iterations
/// @author Willem Deconinck
/// @author Tiago Quintino
class solver_actions_API CriterionMaxIterations : public Criterion {

public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  CriterionMaxIterations ( const std::string& name );

  /// Virtual destructor
  virtual ~CriterionMaxIterations();

  /// Get the class name
  static std::string type_name () { return "CriterionMaxIterations"; }

  /// Simulates this model
  virtual bool operator()();

private:

  /// component where to access the current iteration
  Handle<Component> m_iter_comp;

};

////////////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

#endif // cf3_solver_actions_CriterionMaxIterations_hpp
