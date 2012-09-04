// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_Model_hpp
#define cf3_sdm_Model_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/Model.hpp"

namespace cf3 {
namespace common { class ActionDirector; }
namespace mesh { class Dictionary; }
namespace solver { class Time; class TimeStepping; class History; }
namespace sdm {

// Forward declarations
class BoundaryConditions;
class DomainDiscretization;
class Solver;
class TimeIntegrationStepComputer;
class ComputeLNorm;

////////////////////////////////////////////////////////////////////////////////

/// @brief Spectral Finite Difference iterative solver
///
/// Spectral Finite Difference solver,
/// combining a forward euler time marching scheme with
/// a high-order spectral finite difference spatial scheme
/// @author Willem Deconinck
class sdm_API Model : public solver::Model {

public: // functions

  /// Contructor
  /// @param name of the component
  Model ( const std::string& name );

  /// Virtual destructor
  virtual ~Model();

  /// Get the class name
  static std::string type_name () { return "Model"; }

  Handle<mesh::Dictionary> create_solution_space(const Uint& order, const std::vector<Handle<common::Component> > &regions);

public: // signals

  void signal_create_solution_space( common::SignalArgs& args);
  void signature_create_solution_space( common::SignalArgs& args);

  void signal_set_time_integration( common::SignalArgs& args);
  void signature_set_time_integration( common::SignalArgs& args);

  void signal_create_field( common::SignalArgs& args );
  void signature_create_field( common::SignalArgs& args );

  void signal_add_probe(common::SignalArgs& args);
  void signature_add_probe(common::SignalArgs& args);

public: // functions

  void set_time_integration_scheme(const std::string& type);

  void set_time_integration_step(const std::string& type);

  void set_time_integration_solver(const std::string& solver);

private:

  void config_solution();

  void build_faces(cf3::mesh::Mesh &mesh);

private: // data

  Handle<solver::Time>                      m_time;
  Handle<solver::TimeStepping>              m_time_stepping;
  Handle<sdm::DomainDiscretization>         m_domain_discretization;
  Handle<sdm::BoundaryConditions>           m_boundary_conditions;
  Handle<common::Group>                     m_time_integration;
  Handle<common::Component>                 m_time_integration_scheme;
  Handle<sdm::Solver>                       m_time_integration_solver;
  Handle<sdm::TimeIntegrationStepComputer>  m_time_integration_step;
  Handle<mesh::Dictionary>                  m_solution_space;
  Handle<mesh::Field>                       m_solution;
  Handle<common::ActionDirector>            m_pre_update;
  Handle<common::ActionDirector>            m_post_update;
  Handle<common::ActionDirector>            m_pre_iteration;
  Handle<common::ActionDirector>            m_post_iteration;
  Handle<solver::History>                   m_history;
  Handle<sdm::ComputeLNorm>                 m_residual_norm_computer;
};

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_time_integration_solver_hpp
