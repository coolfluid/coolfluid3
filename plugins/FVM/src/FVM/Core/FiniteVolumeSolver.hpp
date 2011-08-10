// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_Core_FiniteVolumeSolver_hpp
#define CF_FVM_Core_FiniteVolumeSolver_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/CSolver.hpp"

#include "FVM/Core/ComputeUpdateCoefficient.hpp"
#include "FVM/Core/LibCore.hpp"

namespace CF {

namespace Common
{
  class CAction;
  class CLink;
}

namespace Physics { class PhysModel; }

namespace Solver
{
  namespace Actions {  class CIterate; }
  class CTime;
}

namespace Mesh
{
  class Field;
  class CRegion;
  class CDomain;
}

namespace FVM {
namespace Core {

  class ComputeUpdateCoefficient;
  class UpdateSolution;

////////////////////////////////////////////////////////////////////////////////

/// FiniteVolume iterative solver
/// @author Willem Deconinck
class FVM_Core_API FiniteVolumeSolver : public Solver::CSolver {

public: // typedefs

  typedef boost::shared_ptr<FiniteVolumeSolver> Ptr;
  typedef boost::shared_ptr<FiniteVolumeSolver const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  FiniteVolumeSolver ( const std::string& name );

  /// Virtual destructor
  virtual ~FiniteVolumeSolver();

  /// Get the class name
  static std::string type_name () { return "FiniteVolumeSolver"; }

  // functions specific to the FiniteVolumeSolver component

  virtual void solve();

  /// @name SIGNALS
  //@{

  /// creates a boundary condition
  void signal_create_bc( Common::SignalArgs& args );
  void signature_create_bc( Common::SignalArgs& args );

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

  boost::shared_ptr<Solver::Actions::CIterate> m_iterate;
  boost::shared_ptr<Common::CAction> m_apply_bcs;
  boost::shared_ptr<Common::CAction> m_compute_rhs;
  boost::shared_ptr<ComputeUpdateCoefficient> m_compute_update_coefficient;
  boost::shared_ptr<UpdateSolution> m_update_solution;

  boost::weak_ptr<Physics::PhysModel> m_physical_model;
  boost::weak_ptr<Solver::CTime>          m_time;
  boost::weak_ptr<Mesh::CDomain>        m_domain;
};

////////////////////////////////////////////////////////////////////////////////

} // Core
} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_Core_FiniteVolumeSolver_hpp
