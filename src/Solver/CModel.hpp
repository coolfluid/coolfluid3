// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Solver_CModel_hpp
#define cf3_Solver_CModel_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Solver/LibSolver.hpp"
#include <boost/scoped_ptr.hpp>

namespace cf3 {

  namespace common  { class CGroup; }
  namespace Mesh    { class CDomain; }
  namespace Physics { class PhysModel; }

  namespace Solver {

  class CSolver;

////////////////////////////////////////////////////////////////////////////////

/// CModel is the top most component on a simulation structure
/// CModel now stores:
/// - PhysicalModel
/// - Iterative solver
/// - Discretization
/// @author Martin Vymazal
class Solver_API CModel : public common::Component {

public: // typedefs

  typedef boost::shared_ptr<CModel> Ptr;
  typedef boost::shared_ptr<CModel const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CModel ( const std::string& name );

  /// Virtual destructor
  virtual ~CModel();

  /// Get the class name
  static std::string type_name () { return "CModel"; }

  /// creates a domain in this model
  virtual Mesh::CDomain& create_domain( const std::string& name );

  /// create physics
  /// @param builder name of the CBuilder of the physics
  virtual Physics::PhysModel& create_physics( const std::string& builder );

  /// create solver
  /// @param builder_name name of the CBuilder of the solver
  virtual CSolver& create_solver( const std::string& builder );

  /// gets the domain from this model
  virtual Mesh::CDomain& domain();

  /// gets the physics from this model
  virtual Physics::PhysModel& physics();

  /// gets the solver from this model
  virtual CSolver& solver();

  /// gets the solver from this model
  virtual common::CGroup& tools();

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

} // Solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_Solver_CModel_hpp
