// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_EquilibriumEulerFEM_hpp
#define cf3_UFEM_EquilibriumEulerFEM_hpp

#include "LibUFEMParticles.hpp"
#include "../LSSActionUnsteady.hpp"

namespace cf3 {
namespace UFEM {
namespace particles {

/// Simplified particle velocity solved using the full mass matrix
/// Ferry, J. & Balachandarb, S. A fast Eulerian method for disperse two-phase flow International Journal of Multiphase Flow, {2001}, {27}, 1199-1226
class EquilibriumEulerFEM : public LSSActionUnsteady
{
public: // functions

  /// Contructor
  /// @param name of the component
  EquilibriumEulerFEM ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "EquilibriumEulerFEM"; }

private:

  /// Triggered when an option that requires rebuilding the expressions is changed
  void trigger_set_expression();
  
  Real tau, beta;
};

} // particles
} // UFEM
} // cf3


#endif // cf3_UFEM_EquilibriumEulerFEM_hpp
