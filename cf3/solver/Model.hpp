// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_Model_hpp
#define cf3_solver_Model_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"
#include "solver/LibSolver.hpp"
#include <boost/scoped_ptr.hpp>

namespace cf3 {

  namespace common  { class Group; }
  namespace mesh    { class Domain; }
  namespace physics { class PhysModel; }

  namespace solver {

  class Solver;

////////////////////////////////////////////////////////////////////////////////

/// Model is the top most component on a simulation structure
/// Model now stores:
/// - PhysicalModel
/// - Iterative solver
/// - Discretization
/// @author Martin Vymazal
class solver_API Model : public common::Component {

public: // functions

  /// Contructor
  /// @param name of the component
  Model ( const std::string& name );

  /// Virtual destructor
  virtual ~Model();

  /// Get the class name
  static std::string type_name () { return "Model"; }

  /// creates a domain in this model
  virtual mesh::Domain& create_domain( const std::string& name );

  /// create physics
  /// @param builder name of the Builder of the physics
  virtual physics::PhysModel& create_physics( const std::string& builder );

  /// create solver
  /// @param builder_name name of the Builder of the solver
  virtual Solver& create_solver( const std::string& builder );

  /// gets the domain from this model
  virtual mesh::Domain& domain();

  /// gets the physics from this model
  virtual physics::PhysModel& physics();

  /// gets the solver from this model
  virtual Solver& solver();

  /// gets the solver from this model
  virtual common::Group& tools();

  /// Simulates this model
  virtual void simulate();

  /// Short setup
  /// @param solver_builder_name Name of the builder for the solver
  /// @param physics_builder_name Name of the builder for the physics
  virtual void setup(const std::string& solver_builder_name, const std::string& physics_builder_name);

  /// @name SIGNALS
  //@{

  /// Signature of create physics signal @see signal_create_domain
  void signature_create_physics ( common::SignalArgs& node );
  /// Signal to create the physics
  void signal_create_physics ( common::SignalArgs& node );

  /// Signature of create domain signal @see signal_create_domain
  void signature_create_domain ( common::SignalArgs& node );
  /// Signal to create a domain and load a mesh into it
  void signal_create_domain ( common::SignalArgs& node );

  /// Signature of create domain signal @see signal_create_domain
  void signature_create_solver ( common::SignalArgs& node );
  /// Signal to create a domain and load a mesh into it
  void signal_create_solver ( common::SignalArgs& node );

  /// Signature to easily set up a model
  void signature_setup(common::SignalArgs& node);
  /// Signal to set up the model, i.e. create the domain, solver and physical model
  void signal_setup(common::SignalArgs& node);

  /// Signal to start simulating
  void signal_simulate ( common::SignalArgs& node );

  //@} END SIGNALS


private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;

  /// This function is hooked to the mesh_loaded event.
  /// It checks if the mesh that raised the event is in the domain, and if so
  /// calls the mesh_loaded function of the solvers
  void on_mesh_loaded_event(common::SignalArgs& args);
  void on_mesh_changed_event(common::SignalArgs& args);

};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_Model_hpp
