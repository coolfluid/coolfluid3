// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_NavierStokesPhysics_hpp
#define cf3_UFEM_NavierStokesPhysics_hpp

#include "physics/DynamicModel.hpp"
#include "LibUFEM.hpp"

namespace cf3 {

namespace UFEM {

/// Stores the coefficients for the SUPG model and shares them inside a proto expression through the state
struct SUPGCoeffs
{
  /// Reference velocity magnitude
  Real u_ref;

  /// Dynamic viscosity
  Real mu;

  /// Density
  Real rho;

  /// Inverse density
  Real one_over_rho;

  /// Model coefficients
  Real tau_ps, tau_su, tau_bulk;
};

/// Physical model for the Navier Stokes equations
class UFEM_API NavierStokesPhysics : public physics::DynamicModel
{
public:
  NavierStokesPhysics(const std::string& name);
  
  static std::string type_name () { return "NavierStokesPhysics"; }
  
  /// A reference to the stored properties
  SUPGCoeffs& properties()
  {
    return m_coeffs;
  }
  
  /// Link options the supplied properties
  void link_properties(SUPGCoeffs& props);
  
private:
  void trigger_rho();
  SUPGCoeffs m_coeffs;
  std::vector<SUPGCoeffs*> m_linked_coeffs;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_NavierStokesPhysics_hpp
