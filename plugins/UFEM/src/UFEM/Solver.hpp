// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_Solver_hpp
#define cf3_UFEM_Solver_hpp

#include "common/ActionDirector.hpp"
#include "common/OptionURI.hpp"

#include "mesh/Dictionary.hpp"

#include "solver/SimpleSolver.hpp"

#include "solver/actions/Proto/BlockAccumulator.hpp"
#include "solver/actions/Proto/DirichletBC.hpp"
#include "solver/actions/Proto/SolutionVector.hpp"


#include "LibUFEM.hpp"

namespace cf3 {
namespace solver { namespace actions { class Probe; } }
namespace UFEM {

class InitialConditions;

/// Solver for UFEM problems, allowing dynamic configuration and providing access to
/// * Linear system solver
/// * Physical model
/// * Mesh used
/// * Region to loop over
class UFEM_API Solver : public solver::SimpleSolver
{
public: // functions

  /// Contructor
  /// @param name of the component
  Solver ( const std::string& name );

  virtual ~Solver();

  /// Get the class name
  static std::string type_name () { return "Solver"; }

  /// Create a solver where each LSS only requires a single solve to reach steady state.
  /// An initialization step is added automatically
  /// @param builder_name Builder for the actions to add
  Handle<common::Action> add_direct_solver(const std::string& builder_name);

  /// Create a solver where each LSS only requires a single solve to reach steady state.
  /// An initialization step is added automatically
  /// @param builder_names List of builders for the actions to add
  Handle<common::Action> add_unsteady_solver(const std::string& builder_name);

  /// Create a solver where each LSS only requires a single solve to reach steady state.
  /// An initialization step is added automatically
  /// the unsteady solver is advanced multiple times
  /// @param builder_names List of builders for the actions to add
  Handle<common::Action> add_unsteady_advance_solver(const std::string& builder_name);

  /// Create a solver where each LSS requires a multiple solve to reach steady state.
  /// An initialization step is added automatically
  /// @param builder_names List of builders for the actions to add
  Handle<common::Action> add_iteration_solver(const std::string& builder_name);

  /// Create an initial conditions component
  Handle<InitialConditions> create_initial_conditions();

  /// Create the fields, based on the current solver structure
  void create_fields();

  Handle<solver::actions::Probe> add_probe( const std::string& name, Component& parent, const Handle<mesh::Dictionary>& dict = Handle<mesh::Dictionary>() );

  void signature_add_solver(common::SignalArgs& args);
  void signal_add_direct_solver(common::SignalArgs& args);
  void signal_add_unsteady_solver(common::SignalArgs& args);
  void signal_add_unsteady_advance_solver(common::SignalArgs& args);
  void signal_add_iteration_solver(common::SignalArgs& args);
  void signal_create_initial_conditions(common::SignalArgs& args);
  void signal_create_fields(common::SignalArgs& args);
  void signature_add_probe(common::SignalArgs& args);
  void signal_add_probe(common::SignalArgs& args);

  virtual void mesh_loaded(mesh::Mesh& mesh);
  virtual void mesh_changed(mesh::Mesh& mesh);

  virtual void execute();

private:
  /// Triggered by the "ufem_variables_added" event
  void on_variables_added_event(common::SignalArgs& args);
  /// Helper function to add a concrete solver to the giving parent, configuring its options as needed
  Handle<common::Action> add_solver(const std::string& builder_name, Component& parent);

  Handle<InitialConditions> m_initial_conditions;
  bool m_need_field_creation;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_Solver_hpp
