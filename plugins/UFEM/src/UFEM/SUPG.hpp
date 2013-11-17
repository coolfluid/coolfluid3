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

// Helper to get the transpose of either a vector or a scalar

template<typename T>
inline Eigen::Transpose<T const> transpose(const T& mat)
{
  return mat.transpose();
}

inline Real transpose(const Real val)
{
  return val;
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
    
    const Real umag = compute_tau_su(u, element_nu, dt, tau_su);
    tau_bulk = tau_su*umag*umag;
    //std::cout << "old tau_ps: " << tau_ps << ",  tau_su:" << tau_su << ", old tau_bu: " << tau_bulk << " new tau_bu: " << 1./(tau_su * tr_gij) << std::endl;
  }

  /// Only compute the SUPG coefficient
  template<typename UT, typename NUT>
  void operator()(const UT& u, const NUT& nu_eff, const Real& dt, Real& tau_su) const
  {
    compute_tau_su(u, fabs(nu_eff.value().mean()), dt, tau_su);
  }
  
  /// Only compute the SUPG coefficient, overload for scalar viscosity
  template<typename UT>
  void operator()(const UT& u, const Real& nu_eff, const Real& dt, Real& tau_su) const
  {
    compute_tau_su(u, nu_eff, dt, tau_su);
  }

  template<typename UT>
  Real compute_tau_su(const UT& u, const Real& element_nu, const Real& dt, Real& tau_su) const
  {
    typedef typename UT::EtypeT ElementT;
    typedef mesh::Integrators::GaussMappedCoords<1, ElementT::shape> GaussT;
    
    // cell velocity
    u.compute_values(GaussT::instance().coords.col(0));
    Eigen::Matrix<Real, ElementT::dimensionality, ElementT::dimensionality> gij; // metric tensor
    gij = u.nabla()*u.nabla().transpose();
    const Real c2 = 36.;

    //tau_su = 1. / sqrt((4./(dt*dt)) + (u.eval()*gij*detail::transpose(u.eval()))[0] + c2*element_nu*element_nu*gij.squaredNorm());
    
    const Real umag = std::max(detail::norm(u.eval()), 1e-8);

    const Real h = 2.*umag / (u.eval()*u.nabla()).sum();
    tau_su = 1./((u.eval()*u.nabla()).sum() + 2./dt + (4.*element_nu)/(h*h));

    
    return umag;
  }
};

/// Placeholder for the compute_tau operation. Use as compute_tau(velocity_field, nu_eff_field, u_ref, dt, tau_ps, tau_su, tau_bulk)
static solver::actions::Proto::MakeSFOp<ComputeTau>::type const compute_tau = {};

} // UFEM
} // cf3


#endif // cf3_UFEM_SUPG_hpp
