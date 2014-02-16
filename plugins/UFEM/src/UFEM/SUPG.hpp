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
  ComputeTau() :
    c1(1.),
    c2(4.)
  {
  }

  typedef void result_type;

  /// Compute the coefficients for the full Navier-Stokes equations
  template<typename UT, typename NUT>
  void operator()(const UT& u, const NUT& nu_eff, const Real& dt, Real& tau_ps, Real& tau_su, Real& tau_bulk) const
  {
    // Average viscosity
    const Real element_nu = fabs(nu_eff.value().mean());

    compute_coefficients(u, element_nu, dt, tau_ps, tau_su, tau_bulk);
  }

  /// Only compute the SUPG coefficient
  template<typename UT, typename NUT>
  void operator()(const UT& u, const NUT& nu_eff, const Real& dt, Real& tau_su) const
  {
    Real tau_ps, tau_bu;
    compute_coefficients(u, fabs(nu_eff.value().mean()), dt, tau_ps, tau_su, tau_bu);
  }
  
  /// Only compute the SUPG coefficient, overload for scalar viscosity
  template<typename UT>
  void operator()(const UT& u, const Real& nu_eff, const Real& dt, Real& tau_su) const
  {
    Real tau_ps, tau_bu;
    compute_coefficients(u, nu_eff, dt, tau_ps, tau_su, tau_bu);
  }

  template<typename UT>
  void compute_coefficients(const UT& u, const Real element_nu, const Real& dt, Real& tau_ps, Real& tau_su, Real& tau_bulk) const
  {
    typedef typename UT::EtypeT ElementT;
    static const Uint dim = ElementT::dimension;
    typedef mesh::Integrators::GaussMappedCoords<1, ElementT::shape> GaussT;
    
    u.compute_values(GaussT::instance().coords.col(0));

    // Get the minimal edge length
    Real h_rgn = 1e10;
    for(Uint i = 0; i != ElementT::nb_nodes; ++i)
    {
      for(Uint j = 0; j != ElementT::nb_nodes; ++j)
      {
        if(i != j)
        {
          h_rgn = std::min(h_rgn, (u.support().nodes().row(i) - u.support().nodes().row(j)).squaredNorm());
        }
      }
    }
    h_rgn = sqrt(h_rgn);
    
    const Real umag = detail::norm(u.eval());
    const Real h_ugn = h_rgn;//fabs(2.*umag / (u.eval()*u.nabla()).sum());
    
    const Real tau_adv_inv = (2.*umag)/h_ugn;
    const Real tau_time_inv = 2./dt;
    const Real tau_diff_inv = (element_nu)/(h_rgn*h_rgn);
    
    tau_su = 1./(tau_adv_inv + c1*tau_time_inv + c2*tau_diff_inv);
    tau_ps = tau_su;
    tau_bulk = tau_su*umag*umag;
    
    //std::cout << "tau_su: " << tau_su << ", tau_ps: " << tau_ps << ", tau_bulk: " << tau_bulk << ", h_rgn: " << h_rgn << std::endl;
    
//    Eigen::Matrix<Real, ElementT::dimensionality, ElementT::dimensionality> gij; // metric tensor
//    gij.noalias() = u.nabla()*u.nabla().transpose();
//    const Real c2 = 1.;

//    const Real tau_adv_sq = fabs((u.eval()*gij*detail::transpose(u.eval()))[0]); // Very close 0 but slightly negative sometimes
//    const Real tau_diff = element_nu*element_nu*gij.squaredNorm();

//    tau_su = 1. / sqrt((4./(dt*dt)) + tau_adv_sq + c2*tau_diff);
//    tau_bulk = tau_adv_sq < 1e-13 ? 0 : sqrt(tau_adv_sq)/gij.trace();
  }

  // c1 and c2 parameters as defined in:
  //Trofimova, A. V.; Tejada-Martinez, A. E.; Jansen, K. E. & Lahey, R. T. Direct numerical simulation of turbulent channel flows using a stabilized finite element method Computers & Fluids, 2009, 38, 924-938
  Real c1, c2;
};

/// Type for a compute_tau operation. Use as compute_tau(velocity_field, nu_eff_field, dt, tau_ps, tau_su, tau_bulk)
typedef solver::actions::Proto::MakeSFOp<ComputeTau>::type ComputeTauT;


} // UFEM
} // cf3


#endif // cf3_UFEM_SUPG_hpp
