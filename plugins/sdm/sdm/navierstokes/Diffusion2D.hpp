// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_navierstokes_Diffusion2D_hpp
#define cf3_sdm_navierstokes_Diffusion2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "sdm/DiffusiveTerm.hpp"
#include "sdm/navierstokes/LibNavierStokes.hpp"
#include "Physics/NavierStokes/Cons2D.hpp"
#include "Physics/NavierStokes/Roe2D.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace navierstokes {

template <Uint NB_EQS, Uint NB_DIM>
struct DiffusionPhysData : PhysDataBase<NB_EQS,NB_DIM>
{
  typedef Eigen::Matrix<Real,NB_DIM,NB_EQS> RealMatrixNDIMxNEQS;
  RealMatrixNDIMxNEQS solution_gradient;
};

////////////////////////////////////////////////////////////////////////////////

/*
 * The calculation of the stress-tensor involves the use of gradient of temperature and velocities
 * Latex code for these gradients in function of (rho, rhoU, rhoE) (computed using mathematica)
   \begin{align}
     \nabla(u) =& \frac{\rho  \nabla(\rho u)- \rho u \nabla(\rho) }{\rho ^2} \\
     \nabla(v) =& \frac{\rho  \nabla(\rho v)- \rho v \nabla(\rho) }{\rho ^2} \\
     \nabla(w) =& \frac{\rho  \nabla(\rho w)- \rho w \nabla(\rho) }{\rho ^2} \\
     \nabla(T) =& \frac{(\gamma -1)}{\rho ^3 R}  \left[\nabla(\rho)  \left(-\rho  (\rho E)+ (\rho u)^2+(\rho v)^2+(\rho w)^2\right)\right.\\
                & + \left. \rho  (\rho  \nabla(\rho E)- (\rho u) \nabla(\rho u)-(\rho v) \nabla(\rho v)-(\rho w) \nabla(\rho w) ) \right]
   \end{align}
 */

class sdm_navierstokes_API Diffusion2D : public DiffusiveTerm< DiffusionPhysData<4u,2u> >
{
public:
  static std::string type_name() { return "Diffusion2D"; }
  Diffusion2D(const std::string& name) : DiffusiveTerm< PhysData >(name)
  {

    m_gamma = 1.4;
    m_gamma_minus_1 = m_gamma-1.;
    options().add("gamma",m_gamma)
        .description("Heat capacity ratio")
        .attach_trigger( boost::bind( &Diffusion2D::config_constants, this));

    m_R = 287.05;
    options().add("R",m_R)
        .description("Gas constant")
        .attach_trigger( boost::bind( &Diffusion2D::config_constants, this));

    m_k = 2.601e-2;
    options().add("k",m_k)
        .description("Heat conduction")
        .attach_trigger( boost::bind( &Diffusion2D::config_constants, this));

    m_mu = 1.806e-5;
    options().add("mu",m_mu)
        .description("Dynamic viscosity")
        .attach_trigger( boost::bind( &Diffusion2D::config_constants, this));

    config_constants();

  }

  void config_constants()
  {
    m_gamma = options().value<Real>("gamma");
    m_gamma_minus_1 = m_gamma-1.;
    m_R     = options().value<Real>("R");
    m_k     = options().value<Real>("k");
    m_mu    = options().value<Real>("mu");
    m_Cp    = m_gamma*m_R/m_gamma_minus_1;
  }


  virtual ~Diffusion2D() {}

  virtual void compute_flux(PhysData& data, const RealVectorNDIM& unit_normal,
                            RealVectorNEQS& flux, Real& wave_speed)
  {
    const Real& nx = unit_normal[XX];
    const Real& ny = unit_normal[YY];

    const Real& rho  = data.solution[0];
    const Real& rhou = data.solution[1];
    const Real& rhov = data.solution[2];
    const Real& rhoE = data.solution[3];

    const RealVectorNDIM& grad_rho  = data.solution_gradient.col(0);
    const RealVectorNDIM& grad_rhou = data.solution_gradient.col(1);
    const RealVectorNDIM& grad_rhov = data.solution_gradient.col(2);
    const RealVectorNDIM& grad_rhoE = data.solution_gradient.col(3);

    rho2 = rho*rho;
    rho3 = rho2*rho;
    grad_u = (rho*grad_rhou-rhou*grad_rho)/rho2;
    grad_v = (rho*grad_rhov-rhov*grad_rho)/rho2;
    grad_T = m_gamma_minus_1/(m_R*rho3) * (grad_rho*(-rho*rhoE+rhou*rhou+rhov*rhov)
                                           + rho*(rho*grad_rhoE-rhou*grad_rhou-rhov*grad_rhov));
    two_third_divergence_U = 2./3.*(grad_u[XX] + grad_v[YY]);

    // Viscous stress tensor
    tau_xx = m_mu*(2.*grad_u[XX] - two_third_divergence_U);
    tau_yy = m_mu*(2.*grad_v[YY] - two_third_divergence_U);
    tau_xy = m_mu*(grad_u[YY] + grad_v[XX]);

    // Heat flux
    heat_flux = -m_k*(grad_T[XX]*nx + grad_T[YY]*ny);

    flux[0] = 0.;
    flux[1] = tau_xx*nx + tau_xy*ny;
    flux[2] = tau_xy*nx + tau_yy*ny;
    flux[3] = (tau_xx*rhou + tau_xy*rhov)/rho*nx + (tau_xy*rhou + tau_yy*rhov)/rho*ny - heat_flux;

    // maximum of kinematic viscosity nu and thermal diffusivity alpha
    wave_speed = std::max(m_mu/rho, m_k/(rho*m_Cp));
  }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
  Real m_gamma;         // heat capacity ratio
  Real m_gamma_minus_1; // heat capacity ratio -
  Real m_R;             // gas constant
  Real m_k;             // heat conductivity
  Real m_mu;            // dynamic viscosity
  Real m_Cp;


  // allocations
  Real rho2;
  Real rho3;
  RealVectorNDIM grad_u;
  RealVectorNDIM grad_v;
  RealVectorNDIM grad_T;
  Real two_third_divergence_U;

  // Viscous stress tensor
  Real tau_xx;
  Real tau_yy;
  Real tau_xy;

  // Heat flux
  Real heat_flux;

};

////////////////////////////////////////////////////////////////////////////////

} // navierstokes
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_navierstokes_Diffusion2D_hpp
