// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_NavierStokesOps_hpp
#define CF_UFEM_NavierStokesOps_hpp

#include "Math/MatrixTypes.hpp"

#include "Solver/Actions/Proto/ElementOperations.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"

namespace CF {
namespace UFEM {

/// Stores the coefficients for the SUPG model and shares them inside a proto expression through the state
struct SUPGCoeffs
{
  /// Reference velocity magnitude
  Real u_ref;
  
  /// Kinematic viscosity
  Real nu;
  
  /// Density
  Real rho;
  
  /// Model coefficients
  Real tau_ps, tau_su, tau_bulk;
};

struct ComputeTau
{ 
  /// Dummy result
  typedef void result_type;
  
  template<typename UT>
  void operator()(const UT& u, SUPGCoeffs& coeffs) const
  {
    const Real he=sqrt(4./3.141592654*u.support().volume());
    const Real ree=coeffs.u_ref*he/(2.*coeffs.nu);
    const Real xi=std::max(0.,std::min(ree/3.,1.));
    coeffs.tau_ps = he*xi/(2.*coeffs.u_ref);
    coeffs.tau_bulk = he*coeffs.u_ref/xi;
    
    // Average cell velocity
    const RealVector2 u_avg = u.value().colwise().mean();
    const Real umag = u_avg.norm();
    coeffs.tau_su = 0.;
    if(umag > 1e-10)
    {
      const Real h = 2. * u.support().volume() / (u.support().nodes() * (u_avg / umag)).array().abs().sum();
      Real ree=umag*h/(2.*coeffs.nu);
      Real xi=std::max(0.,std::min(ree/3.,1.));
      coeffs.tau_su = h*xi/(2.*umag);
    }
  }
};

/// Placeholder for the compute_tau operation
static Solver::Actions::Proto::MakeSFOp<ComputeTau>::type const compute_tau = {};

} // UFEM
} // CF


#endif // CF_UFEM_NavierStokesOps_hpp
