// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_Iterate_hpp
#define cf3_solver_Iterate_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/actions/LibActions.hpp"
#include "common/ActionDirector.hpp"

namespace cf3 {
namespace solver {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

/// @brief Action component that iteratively executes all contained actions.
///
/// To stop iterating, the configuration "max_iter" can be specified for the amount
/// of iterations, or a stop-criterion, derived from the type solver::actions::Criterion
///
/// @author Willem Deconinck
class solver_actions_API Iterate : public common::ActionDirector
{
public: // functions

  /// Contructor
  /// @param name of the component
  Iterate ( const std::string& name );

  /// Virtual destructor
  virtual ~Iterate();

  /// Get the class name
  static std::string type_name () { return "Iterate"; }

  /// Simulates this model
  virtual void execute();

  /// access to iteration number
  Uint iter() const { return m_iter; }

protected: // data

  /// count of the iteration
  Uint m_iter;

  /// maximum number of iterations
  Uint m_max_iter;

  /// flag to output iteration info
  bool m_verbose;
};

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_Iterate_hpp
