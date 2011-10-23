// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_CSolver_hpp
#define cf3_solver_CSolver_hpp

#include <boost/scoped_ptr.hpp>

#include "common/Component.hpp"
#include "common/ActionDirector.hpp"

#include "solver/LibSolver.hpp"

namespace cf3 {

namespace mesh { class Domain; class Mesh; class FieldManager; }
namespace physics { class PhysModel; }

namespace solver {

////////////////////////////////////////////////////////////////////////////////

/// solver component class
/// Base class for solvers. By default, actions added through the ActionDirector interface are
/// executed in the configured order. Override the execute function to change behavior.
/// Adds an option to set a domain
/// @author Tiago Quintino
/// @author Willem Deconinck
/// @author Bart Janssens
class solver_API CSolver : public common::ActionDirector {

public: // typedefs

  /// type of pointer to Component
  typedef boost::shared_ptr<CSolver> Ptr;
  /// type of pointer to constant Component
  typedef boost::shared_ptr<CSolver const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CSolver ( const std::string& name );

  /// Virtual destructor
  virtual ~CSolver();

  /// Get the class name
  static std::string type_name () { return "CSolver"; }

  /// Called when a mesh is loaded into the domain that is associated with this solver
  virtual void mesh_loaded(mesh::Mesh& mesh);
  /// Called when a mesh is changed into the domain that is associated with this solver
  virtual void mesh_changed(mesh::Mesh& mesh);

  /// Access to the FieldManager, which is a static subcomponent of CSolver
  mesh::FieldManager& field_manager();

protected:

  /// Checked access to the domain (throws if domain is not properly configured)
  mesh::Domain& domain();

  /// Checked access to the physical model
  physics::PhysModel& physics();

private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

#endif // cf3_solver_CSolver_hpp
