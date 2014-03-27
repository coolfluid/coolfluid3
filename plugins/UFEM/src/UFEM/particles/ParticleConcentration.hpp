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
  typedef Real result_type;
  
  DiscontinuityCapture() : c0(1.)
  {
  }
  
  template<typename UT, typename CT>
  Real operator()(const UT& u, const CT& c)
  {
    typedef typename UT::EtypeT ElementT;
    static const Uint dim = ElementT::dimension;
    typedef mesh::Integrators::GaussMappedCoords<1, ElementT::shape> GaussT;
    typedef Eigen::Matrix<Real, dim, 1> ColVecT;
    
    const Real tol = 1e-8;

//    u.compute_values(GaussT::instance().coords.col(0));
//    c.compute_values(GaussT::instance().coords.col(0));
//    u.support().compute_jacobian(GaussT::instance().coords.col(0));
    
    ColVecT g = c.nabla() * c.value();
    const Real grad_norm = g.norm();
    if(grad_norm < tol)
      return 0;
    const Real hg = 2./((g.transpose()/grad_norm)*c.nabla()).cwiseAbs().sum();
    g /= grad_norm; // gradient unit vector
    const Real u_norm = u.eval().norm();
    const Real alpha = u_norm < tol ? 0.5 : ::fabs((u.eval() * g)[0]/u_norm); // Maximum value if velocity is zero
    const Real eta = 2.*alpha*(1-alpha);
    return 0.5*hg*hg/c0*eta;
  }
  
  Real c0;
};

struct CrosswindDiffusion
{
  typedef Real result_type;
  
  CrosswindDiffusion() : d0(1e-4)
  {
  }
  
  template<typename UT, typename CT>
  Real operator()(const UT& u, const CT& c)
  {
    typedef typename UT::EtypeT ElementT;
    static const Uint dim = ElementT::dimension;
    typedef mesh::Integrators::GaussMappedCoords<1, ElementT::shape> GaussT;
    typedef Eigen::Matrix<Real, dim, 1> ColVecT;
    
//     u.compute_values(GaussT::instance().coords.col(0));
//     c.compute_values(GaussT::instance().coords.col(0));
//     u.support().compute_jacobian(GaussT::instance().coords.col(0));
    
    ColVecT g = c.nabla() * c.value();
    const Real grad_norm = g.norm();
    const Real u_norm = u.eval().norm();
    if(grad_norm < 1e-10 || u_norm < 1e-10)
    {
      return 0.;
    }
    g /= grad_norm;
    const Real hg = 2./(g.transpose()*c.nabla()).cwiseAbs().sum();
    return d0*hg*u_norm;
  }
  
  Real d0;
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

  solver::actions::Proto::MakeSFOp<CrosswindDiffusion>::stored_type m_diff_data;
  solver::actions::Proto::MakeSFOp<CrosswindDiffusion>::reference_type diffusion_coeff;
};

} // particles
} // UFEM
} // cf3


#endif // cf3_UFEM_ParticleConcentration_hpp
