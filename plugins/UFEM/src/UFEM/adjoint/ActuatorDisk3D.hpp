// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_ActuatorDisk3D_hpp
#define cf3_UFEM_ActuatorDisk3D_hpp


#include "../UnsteadyAction.hpp"
#include "solver/actions/Proto/BlockAccumulator.hpp"

#include "LibUFEMAdjoint.hpp"


namespace cf3 {
namespace UFEM {
namespace adjoint {

class UFEM_API ActuatorDisk3D : public UnsteadyAction
{
public:

  /// Contructor
  /// @param name of the component
  ActuatorDisk3D ( const std::string& name );

  virtual ~ActuatorDisk3D();

  /// Get the class name
  static std::string type_name () { return "ActuatorDisk3D"; }

  /// Execute the control of heat transfer coefficient usage (dynamic or static)
  virtual void execute();

private:
  /// Called when the boundary regions are set
  virtual void on_regions_set();

  /// Called when an option that requires a rebuild of the expression is changed
  void trigger_setup();

  Real m_u_in = 0.;
  Real m_area = 0.;
  Real m_th = 0.;
  Real m_zh = 0.;
  Real m_yh = 0.;
  Real m_f = 0.;
  Real m_omega = 0.;
  Real m_a = 0.;
  Real m_ct = 0.;
  Real m_u_mean_disk = 0.;


  cf3::solver::actions::Proto::SystemRHS rhs;
  cf3::solver::actions::Proto::SystemMatrix system_matrix;
};

} // adjoint
} // UFEM
} // cf3


#endif // cf3_UFEM_ActuatorDisk3D_hpp
