// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_SynchronizeFields_hpp
#define cf3_solver_actions_SynchronizeFields_hpp

#include "solver/Action.hpp"

#include "solver/actions/LibActions.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
	
namespace mesh { class Field; }

namespace solver {
namespace actions {

class solver_actions_API SynchronizeFields : public cf3::solver::Action {

public: // functions
  /// Contructor
  /// @param name of the component
  SynchronizeFields ( const std::string& name );

  /// Virtual destructor
  virtual ~SynchronizeFields() {}

  /// Get the class name
  static std::string type_name () { return "SynchronizeFields"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_fields();

private: // data

  std::vector< Handle<mesh::Field> > m_fields;

};

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

#endif // cf3_solver_actions_SynchronizeFields_hpp
