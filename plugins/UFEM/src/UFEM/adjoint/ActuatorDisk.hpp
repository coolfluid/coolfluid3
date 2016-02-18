// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_ActuatorDisk_hpp
#define cf3_UFEM_ActuatorDisk_hpp


#include "solver/Action.hpp"
#include "solver/actions/Proto/BlockAccumulator.hpp"

#include "LibUFEMAdjoint.hpp"


namespace cf3 {
namespace UFEM {
namespace adjoint {

class UFEM_API ActuatorDisk : public solver::Action
{
public:

  /// Contructor
  /// @param name of the component
  ActuatorDisk ( const std::string& name );

  virtual ~ActuatorDisk();

  /// Get the class name
  static std::string type_name () { return "ActuatorDisk"; }

  /// Execute the control of heat transfer coefficient usage (dynamic or static)
  virtual void execute();

private:
  /// Called when the boundary regions are set
  virtual void on_regions_set();

  /// Called when an option that requires a rebuild of the expression is changed
  void trigger_setup();

  /// Called when the mean velocity option is changed
  void trigger_u_mean();

  // Example parameter
  Real U_in = 0.;
  Real A = 0.;
  Real CT = 0.;
  RealVector u_mean_in;
  Real Timestep = 0.;

  cf3::solver::actions::Proto::SystemRHS rhs;
  cf3::solver::actions::Proto::SystemMatrix system_matrix;
};

} // adjoint
} // UFEM
} // cf3


#endif // cf3_UFEM_ActuatorDisk_hpp
