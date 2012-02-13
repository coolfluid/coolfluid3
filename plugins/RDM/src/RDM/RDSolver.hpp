// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_solver_hpp
#define cf3_RDM_solver_hpp

#include "common/Group.hpp"

#include "solver/Solver.hpp"
#include "solver/Action.hpp"

#include "RDM/Tags.hpp"

namespace cf3 {

namespace mesh    { class Field;    class Mesh; }
namespace physics { class PhysModel; class Variables; }
namespace solver  { namespace actions { class SynchronizeFields; } }

namespace RDM {


class BoundaryConditions;
class InitialConditions;
class DomainDiscretization;
class IterativeSolver;
class TimeStepping;

////////////////////////////////////////////////////////////////////////////////

/// RDM Solver
///
/// @author Tiago Quintino
/// @author Martin Vymazal
/// @author Mario Ricchiuto
/// @author Willem Deconinck

class RDM_API RDSolver : public cf3::solver::Solver {

public: // typedefs

  /// @todo temporary setting, remove when done
  // false: geo, true: sol
  bool switch_to_sol;


public: // functions

  /// Contructor
  /// @param name of the component
  RDSolver ( const std::string& name );

  /// Virtual destructor
  virtual ~RDSolver();

  /// Get the class name
  static std::string type_name () { return "RDSolver"; }

  // functions specific to the Solver component

  /// solves the PDE's
  virtual void execute();

  /// @return subcomponent for initial conditions
  InitialConditions&    initial_conditions();
  /// @return subcomponent for boundary conditions
  BoundaryConditions&   boundary_conditions();
  /// @return subcomponent for domain terms
  DomainDiscretization& domain_discretization();
  /// @return subcomponent for non linear iterative steps
  IterativeSolver&      iterative_solver();
  /// @return subcomponent for time stepping
  TimeStepping&         time_stepping();
  /// @return subcomponent to prepare mesh for solving
  common::ActionDirector&      prepare_mesh();

  /// @returns the group of shared actions
  common::Group& actions();
  /// @returns the group of shared fields
  common::Group& fields();

  /// @name SIGNALS
  //@{

  //@} END SIGNALS

private: // helper functions

  /// Triggered when physical model is configured
  void config_physics();

  /// Triggered when the mesh is configured
  void config_mesh();

  /// Triggered when the event mesh_changed
  void on_mesh_changed_event( common::SignalArgs& args );

private: // data

  Handle< common::Group > m_actions;  ///< the group of shared actions

  Handle< common::Group > m_fields;   ///< the group of fields

  Handle<InitialConditions>    m_initial_conditions;    ///< subcomponent for initial conditions

  Handle<BoundaryConditions>   m_boundary_conditions;   ///< subcomponent for boundary conditions

  Handle<DomainDiscretization> m_domain_discretization; ///< subcomponent for domain terms

  Handle<IterativeSolver>      m_iterative_solver;      ///< subcomponent for non linear iterative steps

  Handle<TimeStepping>         m_time_stepping;         ///< subcomponent for time stepping

  Handle<ActionDirector>      m_prepare_mesh;          ///< subcomponent that setups the fields

  Handle< physics::PhysModel >   m_physical_model;        ///< physical model

  Handle<mesh::Mesh> m_mesh; ///< mesh which this solver operates

};

////////////////////////////////////////////////////////////////////////////////


} // RDM
} // cf3

#endif // cf3_RDM_solver_hpp
