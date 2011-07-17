// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_Solver_hpp
#define CF_RDM_Solver_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLink.hpp"

#include "Solver/CSolver.hpp"
#include "Solver/Action.hpp"

#include "RDM/Core/LibCore.hpp"

namespace CF {

namespace Mesh    { class CField; class CMesh; }
namespace Physics { class PhysModel; class Variables; }

namespace RDM {
namespace Core {

////////////////////////////////////////////////////////////////////////////////

/// RDM Solver
///
/// @author Tiago Quintino
/// @author Martin Vymazal
/// @author Mario Ricchiuto
/// @author Willem Deconinck

class RDM_Core_API Solver : public CF::Solver::CSolver {

public: // typedefs

  typedef boost::shared_ptr<Solver> Ptr;
  typedef boost::shared_ptr<Solver const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  Solver ( const std::string& name );

  /// Virtual destructor
  virtual ~Solver();

  /// Get the class name
  static std::string type_name () { return "Solver"; }

  // functions specific to the Solver component

  /// solves the PDE's
  virtual void execute();

  /// @name SIGNALS
  //@{

  /// signature for @see signal_initialize_solution
  void signature_signal_initialize_solution( Common::SignalArgs& node );
  /// initializes the solution
  void signal_initialize_solution( Common::SignalArgs& xml );

  /// signature for @see signal_create_boundary_term
  void signature_signal_create_boundary_term( Common::SignalArgs& node );
  /// creates a boundary term
  void signal_create_boundary_term( Common::SignalArgs& xml );

  /// signature for @see signal_create_boundary_term
  void signature_signal_create_domain_term( Common::SignalArgs& node );
  /// creates a domain term
  void signal_create_domain_term( Common::SignalArgs& xml );

  //@} END SIGNALS


private: // functions

  /// called when domain option is set in the solver
  void config_domain();
  /// called when mesh option is set in the solver
  void config_mesh();
  /// called when variables are configured
  void config_physics();

private: // data

  /// physical model discretized by this solver
  boost::weak_ptr< Physics::PhysModel > m_physical_model;

  /// mesh which this solver operates
  boost::weak_ptr<Mesh::CMesh> m_mesh;

  /// solution field pointer
  boost::weak_ptr<Mesh::CField> m_solution;
  /// residual field pointer
  boost::weak_ptr<Mesh::CField> m_residual;
  /// wave_speed field pointer
  boost::weak_ptr<Mesh::CField> m_wave_speed;

  /// action to compute the boundary face terms
  Common::CAction::Ptr m_compute_boundary_terms;
  /// action to compute the domain cell terms
  Common::CAction::Ptr m_compute_domain_terms;

  /// action to update the solution
  CF::Solver::Action::Ptr m_update_solution;
  /// compute the L-norm for convergence
  Common::CAction::Ptr m_compute_norm;
  /// action to cleanup
  CF::Solver::Action::Ptr m_cleanup;
  /// action to iterate solution
  CF::Solver::Action::Ptr m_time_stepping;

};

////////////////////////////////////////////////////////////////////////////////

} // Core
} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_Solver_hpp
