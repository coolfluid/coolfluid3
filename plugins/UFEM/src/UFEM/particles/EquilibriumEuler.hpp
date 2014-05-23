// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_EquilibriumEuler_hpp
#define cf3_UFEM_EquilibriumEuler_hpp

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "LibUFEMParticles.hpp"

namespace cf3 {
namespace UFEM {
namespace particles {

/// Compute the particle velocity field, following
/// Ferry, J. & Balachandar, S. A fast Eulerian method for disperse two-phase flow, International Journal of Multiphase Flow, {2001}, {27}, 1199-1226
class EquilibriumEuler : public solver::actions::Proto::ProtoAction
{
public: // functions

  /// Contructor
  /// @param name of the component
  EquilibriumEuler ( const std::string& name );
  
  virtual ~EquilibriumEuler();

  /// Get the class name
  static std::string type_name () { return "EquilibriumEuler"; }
  
  virtual void execute();

private:
  virtual void on_regions_set();
  void trigger_time();
  void trigger_timestep();

  Handle<common::Action> m_velocity_gradient;
  Handle<solver::Time> m_time;

  struct VelocityFunctor;
  boost::scoped_ptr<VelocityFunctor> m_functor;
};

} // particles
} // UFEM
} // cf3


#endif // cf3_UFEM_EquilibriumEuler_hpp
