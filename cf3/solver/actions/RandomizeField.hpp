// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_RandomizeField_hpp
#define cf3_solver_actions_RandomizeField_hpp

#include "common/Action.hpp"
#include "solver/actions/LibActions.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

class solver_actions_API RandomizeField : public common::Action
{
public: // functions
  /// Contructor
  /// @param name of the component
  RandomizeField ( const std::string& name );

  /// Virtual destructor
  virtual ~RandomizeField() {}

  /// Get the class name
  static std::string type_name () { return "RandomizeField"; }

  /// execute the action
  virtual void execute ();
};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_RandomizeField_hpp
