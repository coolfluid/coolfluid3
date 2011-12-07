// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_CCriterion_hpp
#define cf3_solver_CCriterion_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/actions/LibActions.hpp"
#include "common/Action.hpp"

namespace cf3 {
namespace solver {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

/// CCriterion models a Unsteady PDE problem
/// @author Tiago Quintino
class solver_actions_API CCriterion : public common::Component {

public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  CCriterion ( const std::string& name );

  /// Virtual destructor
  virtual ~CCriterion();

  /// Get the class name
  static std::string type_name () { return "CCriterion"; }

  /// Simulates this model
  virtual bool operator()() = 0;
};

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_CCriterion_hpp
