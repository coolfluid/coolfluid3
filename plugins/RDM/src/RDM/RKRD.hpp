// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_RKRD_hpp
#define CF_RDM_RKRD_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLink.hpp"

#include "Solver/CSolver.hpp"

#include "RDM/Action.hpp"

namespace CF {

namespace Mesh   { class CField; class CMesh; }

namespace RDM {

////////////////////////////////////////////////////////////////////////////////

/// RKRD solver
///
/// @author Tiago Quintino
/// @author Martin Vymazal
/// @author Mario Ricchiuto
/// @author Willem Deconinck

class RDM_API RKRD : public Solver::CSolver {

public: // typedefs

  typedef boost::shared_ptr<RKRD> Ptr;
  typedef boost::shared_ptr<RKRD const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  RKRD ( const std::string& name );

  /// Virtual destructor
  virtual ~RKRD();

  /// Get the class name
  static std::string type_name () { return "RKRD"; }

  // functions specific to the RKRD component
  
  /// solves the PDE's
  virtual void solve();
  
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

  void config_domain();
  void config_mesh();

private: // data

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
  RDM::Action::Ptr m_update_solution;
  /// compute the L-norm for convergence
  Common::CAction::Ptr m_compute_norm;
  /// action to cleanup
  RDM::Action::Ptr m_cleanup;
  /// action to iterate solution
  RDM::Action::Ptr m_time_stepping;

};

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_RKRD_hpp
