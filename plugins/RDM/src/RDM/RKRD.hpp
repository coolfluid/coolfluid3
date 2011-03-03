// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_RKRD_hpp
#define CF_RDM_RKRD_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLink.hpp"
#include "Common/CAction.hpp"

#include "Solver/CSolver.hpp"

#include "RDM/LibRDM.hpp"

namespace CF {

namespace Mesh   { class CField2; class CMesh; }

namespace RDM {

////////////////////////////////////////////////////////////////////////////////

/// RKRD solver
/// @author Tiago Quintino
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
  
  virtual void solve();
  
  /// @name SIGNALS
  //@{

  /// signature for @see signal_create_boundary_term
  void signature_signal_create_boundary_term( Common::Signal::arg_t& node );
  /// creates a boundary term
  void signal_create_boundary_term( Common::Signal::arg_t& xml );

  /// signature for @see signal_create_boundary_term
  void signature_create_domain_term( Common::Signal::arg_t& node );
  /// creates a domain term
  void signal_create_domain_term( Common::Signal::arg_t& xml );

  //@} END SIGNALS


private: // functions

  void config_domain();
  void config_mesh();

private: // data

  /// CFL number
  CF::Real m_cfl;

  /// mesh which this solver operates
  boost::weak_ptr<Mesh::CMesh> m_mesh;

  /// solution field pointer
  boost::weak_ptr<Mesh::CField2> m_solution;
  /// residual field pointer
  boost::weak_ptr<Mesh::CField2> m_residual;
  /// wave_speed field pointer
  boost::weak_ptr<Mesh::CField2> m_wave_speed;

  /// action to compute the boundary face terms
  Common::CAction::Ptr m_compute_boundary_face_terms;

  /// action to compute the domain cell terms
  Common::CAction::Ptr m_compute_volume_cell_terms;

  Uint m_nb_iter;
};

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_RKRD_hpp
