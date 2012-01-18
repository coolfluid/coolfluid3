// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_CPrintIterationSummary_hpp
#define cf3_solver_actions_CPrintIterationSummary_hpp

#include "common/Action.hpp"

#include "solver/actions/LibActions.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh   { class Field; }
namespace solver {
namespace actions {

class solver_actions_API CPrintIterationSummary : public common::Action {

public: // typedefs

  /// pointers
  
  

public: // functions
  /// Contructor
  /// @param name of the component
  CPrintIterationSummary ( const std::string& name );

  /// Virtual destructor
  virtual ~CPrintIterationSummary() {}

  /// Get the class name
  static std::string type_name () { return "CPrintIterationSummary"; }

  /// execute the action
  virtual void execute ();

private: // data

  Handle<Component> my_norm;
  Handle<Component> my_iter;

};

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

#endif // cf3_solver_actions_CPrintIterationSummary_hpp
