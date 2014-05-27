//#ifndef SPALARTALLMARAS_H
//#define SPALARTALLMARAS_H

//#endif // SPALARTALLMARAS_H


// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_SpalartAllmaras_hpp
#define cf3_UFEM_SpalartAllmaras_hpp

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/max.hpp>

#define BOOST_PROTO_MAX_ARITY 10
#ifdef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
  #undef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
#endif
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10

#include <boost/scoped_ptr.hpp>

#include "LibUFEM.hpp"
#include "LSSActionUnsteady.hpp"
#include "CrossWindDiffusion.hpp"
#include "SUPG.hpp"

namespace cf3 {

namespace UFEM {

inline Real fv1(const Real chi, const Real cv1)
{
  return chi*chi*chi / (chi*chi*chi + cv1*cv1*cv1);
}
  
struct ComputeSACoeffs
{
  typedef void result_type;

  ComputeSACoeffs() :
    cb1(0.1355),
    cb2(0.622),
    ct3(1.2),
    ct4(0.5),
    cv1(7.1),
    cv2(5.),
    cw2(0.3),
    cw3(2.),
    kappa(0.41),
    sigma(2./3.)
  {
    cw1 = cb1 / (kappa*kappa) + (1. + cb2)/sigma;
  }
  
  template<typename UT, typename NUT, typename DT>
  void operator()(const UT& u, const NUT& nu_t, const DT& wall_dist, const Real& nu_lam)
  {
    typedef mesh::Integrators::GaussMappedCoords<1, UT::SupportT::EtypeT::shape> GaussT;
    u.support().compute_shape_functions(GaussT::instance().coords.col(0));
    u.support().compute_jacobian(GaussT::instance().coords.col(0));
    u.compute_values(GaussT::instance().coords.col(0));
    nu_t.compute_values(GaussT::instance().coords.col(0));
    

    // nu_t.value() is a column vector with the nodal values of the viscosity for the element.
    // mean comes from the Eigen library
    Real nu_t_cell = nu_t.value().mean();
    if(nu_t_cell < 0.)
    {
      nu_t_cell = 0.;
    }
    const Real chi = nu_t_cell / nu_lam;
    
    ft2 = ct3 * ::exp(-ct4*chi*chi);

    // Computing S needs the gradient, which is calculated at a mapped coordinate.
    // We take the first gauss point here, i.e. we approximate the gradient by the value at the cell center.
    // nabla is the gradient matrix of the shape function of u
    Eigen::Matrix<Real, UT::dimension, UT::dimension> nabla_u = u.nabla() * u.value(); // The gradient of the velocity is the shape function gradient matrix multiplied with the nodal values
//     std::cout << "u: " << u.value().transpose() << ", nabla_u:\n" << nabla_u << std::endl;
    // wall distance
    const Real d = wall_dist.value().mean(); // Mean cell wall distance
    const Real omega = sqrt(0.5)*(nabla_u - nabla_u.transpose()).norm();
    
    const Real fv2    = 1. - chi/(1. + chi*fv1(chi, cv1));
    const Real Sbar = nu_t_cell / (kappa*kappa*d*d)*fv2;
    const Real c2 = 0.7;
    const Real c3 = 0.9;
    //std::cout << "omega: " << omega << std::endl;
    if(Sbar >= -c2*omega)
      Stilde = omega + Sbar;
    else
      Stilde = omega + omega*(c2*c2*omega + c3*Sbar) / ((c3 - 2*c2)*omega - Sbar);
    
    Real r = nu_t_cell/(Stilde*kappa*kappa*d*d);
    if (!std::isfinite(r) || r > 10)
      r = 10;
    const Real g      = r+cw2*(r*r*r*r*r*r-r);
    const Real g6 = g*g*g*g*g*g;
    const Real cw36   = cw3*cw3*cw3*cw3*cw3*cw3;
    fw     = g*::pow((1.+cw36)/(g6 + cw36),1./6.);

    //std::cout << "sa_params: " << ft2 << ", " << Stilde << ", " << fw << ", " << diag_diff << std::endl;
  }
  
  // Model constants
  Real cb1;
  Real cb2;
  Real ct3;
  Real ct4;
  Real cv1;
  Real cv2;
  Real cw2;
  Real cw3;
  Real kappa;
  Real sigma;
  
  Real cw1;
  
  // Output coefficients
  Real ft2;
  Real Stilde;
  Real fw;
};

/// solver for SpalartAllmaras turbulence model
class UFEM_API SpalartAllmaras : public LSSActionUnsteady
{
public: // functions

  /// Contructor
  /// @param name of the component
  SpalartAllmaras ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "SpalartAllmaras"; }

private:

  /// Ensure the automatic creation of initial conditions
  virtual void on_initial_conditions_set(InitialConditions& initial_conditions);

  void trigger_set_expression();

  Real tau_su;

  ComputeTau compute_tau;
  solver::actions::Proto::MakeSFOp<ComputeSACoeffs>::stored_type m_sa_coeff;
  solver::actions::Proto::MakeSFOp<ComputeSACoeffs>::reference_type comp_sa;

  solver::actions::Proto::MakeSFOp<CrosswindDiffusion>::stored_type m_diff_data;
  solver::actions::Proto::MakeSFOp<CrosswindDiffusion>::reference_type diffusion_coeff;
};

} // UFEM
} // cf3

#endif // cf3_UFEM_NavierStokes_hpp
