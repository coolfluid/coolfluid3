// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_ParticleConcentration_hpp
#define cf3_UFEM_ParticleConcentration_hpp

#include "solver/actions/Proto/ElementOperations.hpp"

#include "LibUFEMParticles.hpp"
#include "../LSSActionUnsteady.hpp"
#include "../SUPG.hpp"

namespace cf3 {
namespace UFEM {
namespace particles {

/// Stabilization as described by Tezduyar et al.
struct DiscontinuityCapture
{
  typedef void result_type;
  
  DiscontinuityCapture() : c0(1.)
  {
  }
  
  template<typename UT, typename CT>
  void operator()(const UT& u, const CT& c, Real& tau_dc)
  {
    typedef typename UT::EtypeT ElementT;
    static const Uint dim = ElementT::dimension;
    typedef mesh::Integrators::GaussMappedCoords<1, ElementT::shape> GaussT;
    typedef Eigen::Matrix<Real, dim, 1> ColVecT;
    
    u.compute_values(GaussT::instance().coords.col(0));
    c.compute_values(GaussT::instance().coords.col(0));
    u.support().compute_jacobian(GaussT::instance().coords.col(0));
    
    ColVecT g = c.nabla() * c.value();
    const Real grad_norm = g.norm();
    const Real u_norm = u.eval().norm();
    if(grad_norm < 1e-10 || u_norm < 1e-10)
    {
      tau_dc = 0.;
      return;
    }
    g /= grad_norm;
    const Real ug = u.eval()*g;
    //const Real hg = 1./::sqrt((u.support().jacobian_inverse().transpose() * u.support().jacobian_inverse()).norm());
    const Real hg = 2./(g.transpose()*c.nabla()).cwiseAbs().sum();
    //const Real hg = 1./sqrt((g.transpose() * (u.support().jacobian_inverse().transpose() * u.support().jacobian_inverse()) * g)[0]);
    const Real ugu = ::fabs(ug)/u_norm;
    const Real eta = 2.*(1.-ugu)*ugu;
    tau_dc = 0.5*hg*hg/c0*(ug>0 ? 1. : -1.);
  }
  
  Real c0;
};
  
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

  /// Stabilization coefficients
  Real tau_su, tau_dc;
  /// Theta parameter for the theta-scheme
  Real m_theta;

  ComputeTau compute_tau;
  solver::actions::Proto::MakeSFOp<DiscontinuityCapture>::stored_type m_capt_data;
  solver::actions::Proto::MakeSFOp<DiscontinuityCapture>::reference_type discontinuity_capture;
};

} // particles
} // UFEM
} // cf3


#endif // cf3_UFEM_ParticleConcentration_hpp
