// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_ParticleConcentration_hpp
#define cf3_UFEM_ParticleConcentration_hpp

#include "LibUFEMParticles.hpp"
#include "../LSSActionUnsteady.hpp"
#include "../SUPG.hpp"

namespace cf3 {
namespace UFEM {
namespace particles {

/// Particle concentration transport, following
/// Ferry, J. & Balachandarb, S. A fast Eulerian method for disperse two-phase flow International Journal of Multiphase Flow, {2001}, {27}, 1199-1226
class ParticleConcentration : public LSSActionUnsteady
{
public: // functions

  /// Contructor
  /// @param name of the component
  ParticleConcentration ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "ParticleConcentration"; }

private:

  /// Triggered when an option that requires rebuilding the expressions is changed
  void trigger_set_expression();

  /// Stabilization coefficient
  Real tau_su;
  /// Theta parameter for the theta-scheme
  Real m_theta;

  ComputeTau compute_tau;
};

} // particles
} // UFEM
} // cf3


#endif // cf3_UFEM_ParticleConcentration_hpp
