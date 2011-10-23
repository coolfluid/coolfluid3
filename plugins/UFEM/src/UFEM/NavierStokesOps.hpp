// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_NavierStokesOps_hpp
#define cf3_UFEM_NavierStokesOps_hpp

#include "math/MatrixTypes.hpp"

#include "solver/Actions/Proto/ElementOperations.hpp"
#include "solver/Actions/Proto/Terminals.hpp"

namespace cf3 {
namespace UFEM {

/// Stores the coefficients for the SUPG model and shares them inside a proto expression through the state
struct SUPGCoeffs
{
  /// Reference velocity magnitude
  Real u_ref;

  /// Kinematic viscosity
  Real mu;

  /// Density
  Real rho;
  
  /// Inverse density
  Real one_over_rho;

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
    apply(boost::mpl::int_<UT::dimension>(), u, coeffs);
  }
  
  /// Static dispatch for 1D case
  template<typename UT>
  void apply(const boost::mpl::int_<1>, const UT& u, SUPGCoeffs& coeffs) const
  {
    throw common::NotImplemented(FromHere(), "1D PSPG and SUPG stabilization is not implemented");
  }
  
  /// Static dispatch for 2D case
  template<typename UT>
  void apply(const boost::mpl::int_<2>, const UT& u, SUPGCoeffs& coeffs) const
  {
    const Real he=sqrt(4./3.141592654*u.support().volume());
    const Real ree=coeffs.rho*coeffs.u_ref*he/(2.*coeffs.mu);
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
      Real ree=coeffs.rho*umag*h/(2.*coeffs.mu);
      Real xi=std::max(0.,std::min(ree/3.,1.));
      coeffs.tau_su = h*xi/(2.*umag);
    }
  }
  
  /// Static dispatch for 3D case
  template<typename UT>
  void apply(const boost::mpl::int_<3>, const UT& u, SUPGCoeffs& coeffs) const
  {
    throw common::NotImplemented(FromHere(), "3D PSPG and SUPG stabilization is not implemented");
  }
};

/// Placeholder for the compute_tau operation
static solver::Actions::Proto::MakeSFOp<ComputeTau>::type const compute_tau = {};

} // UFEM
} // cf3


#endif // cf3_UFEM_NavierStokesOps_hpp
