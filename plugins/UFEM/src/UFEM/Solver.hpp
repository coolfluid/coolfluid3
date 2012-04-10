// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_Solver_hpp
#define cf3_UFEM_Solver_hpp

#include "common/ActionDirector.hpp"
#include "common/OptionURI.hpp"

#include "solver/SimpleSolver.hpp"

#include "solver/actions/Proto/BlockAccumulator.hpp"
#include "solver/actions/Proto/DirichletBC.hpp"
#include "solver/actions/Proto/SolutionVector.hpp"

#include "LibUFEM.hpp"

namespace cf3 {

namespace UFEM {

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
  
  /// Create an initial conditions component
  Handle<common::ActionDirector> create_initial_conditions();
  
  void signature_add_solver(common::SignalArgs& args);
  void signal_add_direct_solver(common::SignalArgs& args);
  void signal_add_unsteady_solver(common::SignalArgs& args);
  void signal_create_initial_conditions(common::SignalArgs& args);

  virtual void mesh_loaded(mesh::Mesh& mesh);
  virtual void mesh_changed(mesh::Mesh& mesh);
};

} // UFEM
} // cf3


#endif // cf3_UFEM_Solver_hpp
