// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_SFDSolver_hpp
#define cf3_SFDM_SFDSolver_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/CSolver.hpp"

#include "SFDM/PrepareMesh.hpp"
#include "SFDM/TimeStepping.hpp"
#include "SFDM/IterativeSolver.hpp"
#include "SFDM/InitialConditions.hpp"
#include "SFDM/DomainDiscretization.hpp"
#include "SFDM/BoundaryConditions.hpp"

namespace cf3 {

namespace common    { class Group; }
namespace solver    { namespace actions { class CSynchronizeFields; } }
//namespace RiemannSolvers { class RiemannSolver; }
namespace SFDM {

// Forward declarations
class PrepareMesh;
class BoundaryConditions;
class InitialConditions;
class DomainDiscretization;
class IterativeSolver;
class TimeStepping;
class SharedCaches;

////////////////////////////////////////////////////////////////////////////////

/// @brief Spectral Finite Difference iterative solver
///
/// Spectral Finite Difference solver,
/// combining a forward euler time marching scheme with
/// a high-order spectral finite difference spatial scheme
/// @author Willem Deconinck
class SFDM_API SFDSolver : public solver::CSolver {

public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  SFDSolver ( const std::string& name );

  /// Virtual destructor
  virtual ~SFDSolver();

  /// Get the class name
  static std::string type_name () { return "SFDSolver"; }

  // functions specific to the SFDSolver component

  /// solves the PDE's
  virtual void execute();

  /// @return subcomponent for boundary conditions
  BoundaryConditions&   boundary_conditions()    { return *m_boundary_conditions; }
  /// @return subcomponent for initial conditions
  InitialConditions&    initial_conditions()     { return *m_initial_conditions; }
  /// @return subcomponent for domain terms
  DomainDiscretization& domain_discretization()  { return *m_domain_discretization; }
  /// @return subcomponent for time stepping
  TimeStepping&         time_stepping()          { return *m_time_stepping; }
  /// @return subcomponent for non linear iterative steps
  IterativeSolver&      iterative_solver()       { return *m_iterative_solver; }
  /// @return subcomponent to prepare mesh for solving
  PrepareMesh&          prepare_mesh()           { return *m_prepare_mesh; }
  /// @returns the group of shared actions
  common::Group&        actions()                 { return *m_actions; }

  /// @return shared caches
  SharedCaches& shared_caches()                  { return *m_shared_caches; }

  mesh::Mesh& mesh()                             { return *m_mesh; }

  std::vector< Handle<mesh::Region> >& regions() { return m_regions; }

//  RiemannSolvers::RiemannSolver& riemann_solver() { return *m_riemann_solver; }

private: // functions

  /// Triggered when physical model is configured
  void config_physics();

  /// Triggered when the mesh is configured
  void config_mesh();

  /// Triggered when regions is configured
  void config_regions();

  /// Triggered when the event mesh_changed
  void on_mesh_changed_event( common::SignalArgs& args );

private: // data

  bool m_mesh_configured;

  Handle<common::Group>              m_actions;               ///< the group of shared actions
  Handle<common::Group>              m_fields;                ///< the group of fields

  Handle<SharedCaches>               m_shared_caches;
  Handle<physics::PhysModel>         m_physical_model;       ///< physical model
  Handle<mesh::Mesh>                 m_mesh;                 ///< mesh which this solver operates
  std::vector<Handle<mesh::Region> > m_regions;              ///< mesh which this solver operates

  Handle<PrepareMesh>                m_prepare_mesh;          ///< subcomponent that setups the fields
  Handle<TimeStepping>               m_time_stepping;         ///< subcomponent for time stepping
  Handle<IterativeSolver>            m_iterative_solver;      ///< subcomponent for non linear iterative steps
  Handle<DomainDiscretization>       m_domain_discretization; ///< subcomponent for domain terms
  Handle<InitialConditions>          m_initial_conditions;    ///< subcomponent for initial conditions
  Handle<BoundaryConditions>         m_boundary_conditions;   ///< subcomponent for boundary conditions

//  Handle<RiemannSolvers::RiemannSolver> m_riemann_solver;  ///< Riemann solver

};

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_SFDSolver_hpp
