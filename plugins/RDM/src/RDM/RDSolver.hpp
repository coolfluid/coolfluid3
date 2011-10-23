// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_solver_hpp
#define cf3_RDM_solver_hpp

#include "common/Group.hpp"

#include "solver/CSolver.hpp"
#include "solver/Action.hpp"

#include "RDM/Tags.hpp"

namespace cf3 {

namespace mesh    { class Field;    class Mesh; }
namespace Physics { class PhysModel; class Variables; }
namespace solver  { namespace Actions { class CSynchronizeFields; } }

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

class RDM_API RDSolver : public cf3::solver::CSolver {

public: // typedefs

  typedef boost::shared_ptr<RDSolver> Ptr;
  typedef boost::shared_ptr<RDSolver const> ConstPtr;

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

  common::Group::Ptr m_actions;  ///< the group of shared actions

  common::Group::Ptr m_fields;   ///< the group of fields

  boost::shared_ptr<InitialConditions>    m_initial_conditions;    ///< subcomponent for initial conditions

  boost::shared_ptr<BoundaryConditions>   m_boundary_conditions;   ///< subcomponent for boundary conditions

  boost::shared_ptr<DomainDiscretization> m_domain_discretization; ///< subcomponent for domain terms

  boost::shared_ptr<IterativeSolver>      m_iterative_solver;      ///< subcomponent for non linear iterative steps

  boost::shared_ptr<TimeStepping>         m_time_stepping;         ///< subcomponent for time stepping

  boost::shared_ptr<ActionDirector>      m_prepare_mesh;          ///< subcomponent that setups the fields

  boost::weak_ptr< Physics::PhysModel >   m_physical_model;        ///< physical model

  boost::weak_ptr<mesh::Mesh> m_mesh; ///< mesh which this solver operates

};

////////////////////////////////////////////////////////////////////////////////


} // RDM
} // cf3

#endif // cf3_RDM_solver_hpp
