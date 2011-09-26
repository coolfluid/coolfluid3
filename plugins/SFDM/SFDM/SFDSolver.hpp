// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_SFDSolver_hpp
#define CF_SFDM_SFDSolver_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/CSolver.hpp"

#include "SFDM/PrepareMesh.hpp"
#include "SFDM/TimeStepping.hpp"
#include "SFDM/IterativeSolver.hpp"
#include "SFDM/InitialConditions.hpp"
#include "SFDM/DomainDiscretization.hpp"

namespace CF {

namespace Common    { class CGroup; }
namespace Solver    { namespace Actions { class CSynchronizeFields; } }

namespace SFDM {

// Forward declarations
class PrepareMesh;
class BoundaryConditions;
class InitialConditions;
class DomainDiscretization;
class IterativeSolver;
class TimeStepping;

////////////////////////////////////////////////////////////////////////////////

/// @brief Spectral Finite Difference iterative solver
///
/// Spectral Finite Difference solver,
/// combining a forward euler time marching scheme with
/// a high-order spectral finite difference spatial scheme
/// @author Willem Deconinck
class SFDM_API SFDSolver : public Solver::CSolver {

public: // typedefs

  typedef boost::shared_ptr<SFDSolver> Ptr;
  typedef boost::shared_ptr<SFDSolver const> ConstPtr;

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

#if 0
  /// @return subcomponent for boundary conditions
  BoundaryConditions&   boundary_conditions();
#endif

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
  Common::CGroup&       actions()                { return *m_actions; }
  /// @returns the group of shared fields
  Common::CGroup&       fields()                 { return *m_fields; }

  Mesh::CMesh& mesh() { return *m_mesh.lock(); }

private: // functions

  /// Triggered when physical model is configured
  void config_physics();

  /// Triggered when the mesh is configured
  void config_mesh();

  /// Triggered when the event mesh_changed
  void on_mesh_changed_event( Common::SignalArgs& args );

private: // data

  bool m_mesh_configured;

  boost::shared_ptr<Common::CGroup>          m_actions;               ///< the group of shared actions
  boost::shared_ptr<Common::CGroup>          m_fields;                ///< the group of fields

  boost::weak_ptr<Physics::PhysModel>        m_physical_model;        ///< physical model
  boost::weak_ptr<Mesh::CMesh>               m_mesh;                  ///< mesh which this solver operates

  boost::shared_ptr<PrepareMesh>             m_prepare_mesh;          ///< subcomponent that setups the fields
  boost::shared_ptr<TimeStepping>            m_time_stepping;         ///< subcomponent for time stepping
  boost::shared_ptr<IterativeSolver>         m_iterative_solver;      ///< subcomponent for non linear iterative steps
  boost::shared_ptr<DomainDiscretization>    m_domain_discretization; ///< subcomponent for domain terms
  boost::shared_ptr<InitialConditions>       m_initial_conditions;    ///< subcomponent for initial conditions
//  boost::shared_ptr<BoundaryConditions>   m_boundary_conditions;   ///< subcomponent for boundary conditions

};

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_SFDM_SFDSolver_hpp
