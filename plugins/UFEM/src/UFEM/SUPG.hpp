// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_SUPG_hpp
#define cf3_UFEM_SUPG_hpp

#include "math/MatrixTypes.hpp"

#include "solver/actions/Proto/ElementOperations.hpp"
#include <solver/actions/Proto/ElementData.hpp>

#include "NavierStokesPhysics.hpp"

namespace cf3 {

namespace UFEM {

namespace detail
{
// Helper to get the norm of either a vector or a scalar

template<typename T>
inline Real norm(const T& vector)
{
  return vector.norm();
}

inline Real norm(const Real scalar)
{
  return scalar;
}

}

/// Calculation of the stabilization coefficients for the SUPG method
struct ComputeTau
{
  typedef void result_type;

  /// Compute the coefficients for the full Navier-Stokes equations
  template<typename UT, typename NUT>
  void operator()(const UT& u, const NUT& nu_eff, const Real& u_ref, const Real& dt, Real& tau_ps, Real& tau_su, Real& tau_bulk) const
  {
    typedef typename UT::EtypeT ElementT;

    // Average viscosity
    Real element_nu = fabs(nu_eff.value().mean());

    const Real he = UT::dimension == 2 ? sqrt(4./3.141592654*u.support().volume()) : ::pow(3./4./3.141592654*u.support().volume(),1./3.);
    const Real ree=u_ref*he/(2.*element_nu);
    cf3_assert(ree > 0.);
    const Real xi = ree < 3. ? 0.3333333333333333*ree : 1.;
    tau_ps = he*xi/(2.*u_ref);
    tau_bulk = he*u_ref/xi;
    tau_su = compute_tau_su(u, element_nu, dt);
  }

  /// Only compute the SUPG coefficient
  template<typename UT, typename NUT>
  void operator()(const UT& u, const NUT& nu_eff, const Real& dt, Real& tau_su) const
  {
    tau_su = compute_tau_su(u, fabs(nu_eff.value().mean()), dt);
  }
  
  /// Only compute the SUPG coefficient, overload for scalar viscosity
  template<typename UT>
  void operator()(const UT& u, const Real& nu_eff, const Real& dt, Real& tau_su) const
  {
    tau_su = compute_tau_su(u, nu_eff, dt);
  }

  template<typename UT>
  Real compute_tau_su(const UT& u, const Real& element_nu, const Real& dt) const
  {
    typedef typename UT::EtypeT ElementT;
    typedef mesh::Integrators::GaussMappedCoords<1, ElementT::shape> GaussT;


    // cell velocity
    u.compute_values(GaussT::instance().coords.col(0));
    const Real umag = detail::norm(u.eval());

    if(umag > 1e-10)
    {
      const Real h = 2.*umag / (u.eval()*u.nabla()).sum();
      const Real tau_adv = h/(2.*umag);
      const Real tau_time = 0.5*dt;
      const Real tau_diff = h*h/(4.*element_nu);
      return 1./(1./tau_adv + 1./tau_time + 1./tau_diff);
    }

    return 0.;
  }
};

/// Placeholder for the compute_tau operation. Use as compute_tau(velocity_field, nu_eff_field, u_ref, dt, tau_ps, tau_su, tau_bulk)
static solver::actions::Proto::MakeSFOp<ComputeTau>::type const compute_tau = {};

} // UFEM
} // cf3


#endif // cf3_UFEM_SUPG_hpp
