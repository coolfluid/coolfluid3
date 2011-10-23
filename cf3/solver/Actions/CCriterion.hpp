// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_CCriterion_hpp
#define cf3_solver_CCriterion_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/Actions/LibActions.hpp"
#include "common/Action.hpp"

namespace cf3 {
namespace solver {
namespace Actions {

////////////////////////////////////////////////////////////////////////////////

/// CCriterion models a Unsteady PDE problem
/// @author Tiago Quintino
class solver_Actions_API CCriterion : public common::Component {

public: // typedefs

  typedef boost::shared_ptr<CCriterion> Ptr;
  typedef boost::shared_ptr<CCriterion const> ConstPtr;

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

} // Actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_CCriterion_hpp
