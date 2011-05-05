// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_SFDSolver_hpp
#define CF_SFDM_SFDSolver_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/CSolver.hpp"

#include "SFDM/LibSFDM.hpp"

namespace CF {

namespace Common {
  class CAction;
  class CLink;
}
namespace Solver { 
  class CPhysicalModel;
  class CTime;
}

namespace Mesh {
  class CField;
  class CRegion;
  class CDomain;
}

namespace SFDM {

////////////////////////////////////////////////////////////////////////////////

/// @brief Spectral Finite Difference iterative solver
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
  
  virtual void solve();
  
  /// @name SIGNALS
  //@{

  /// creates a boundary condition
  void signal_create_bc( Common::SignalArgs& xml );
  void signature_create_bc( Common::SignalArgs& xml );

  //@} END SIGNALS


  Common::CAction& create_bc(const std::string& name, const std::vector<boost::shared_ptr<Mesh::CRegion> >& regions, const std::string& bc_builder_name);
  Common::CAction& create_bc(const std::string& name, const Mesh::CRegion& region, const std::string& bc_builder_name);

  
private: // functions

  void trigger_domain();

  void trigger_time();

  void trigger_physical_model();
  
  void auto_config_fields(Component& parent);

private: // data
  
  boost::shared_ptr<Common::CLink> m_solution;
  boost::shared_ptr<Common::CLink> m_residual;
  boost::shared_ptr<Common::CLink> m_wave_speed;
  boost::shared_ptr<Common::CLink> m_update_coeff;

  boost::shared_ptr<Common::CAction> m_iterate;
  boost::shared_ptr<Common::CAction> m_apply_bcs;
  boost::shared_ptr<Common::CAction> m_compute_rhs;
  boost::shared_ptr<Common::CAction> m_compute_update_coefficient;
  boost::shared_ptr<Common::CAction> m_update_solution;

  boost::weak_ptr<Solver::CPhysicalModel> m_physical_model;
  boost::weak_ptr<Solver::CTime>          m_time;
  boost::weak_ptr<Mesh::CDomain>          m_domain;
};

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_SFDM_SFDSolver_hpp
