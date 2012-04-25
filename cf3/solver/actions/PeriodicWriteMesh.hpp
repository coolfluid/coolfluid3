// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_PeriodicWriteMesh_hpp
#define cf3_solver_actions_PeriodicWriteMesh_hpp

#include "solver/actions/LibActions.hpp"
#include "solver/Action.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh   { class Field; class Mesh; class WriteMesh; }
namespace solver {
namespace actions {

class solver_actions_API PeriodicWriteMesh : public solver::Action {

public: // functions
  /// Contructor
  /// @param name of the component
  PeriodicWriteMesh ( const std::string& name );

  /// Virtual destructor
  virtual ~PeriodicWriteMesh() {}

  /// Get the class name
  static std::string type_name () { return "PeriodicWriteMesh"; }

  /// execute the action
  virtual void execute ();

private: // data

  Handle<Component> m_iterator;  ///< component that holds the iteration

  mesh::WriteMesh& m_writer; ///< mesh writer

};

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

#endif // cf3_solver_actions_PeriodicWriteMesh_hpp
