// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "FVM/Core/RoeCons1D.hpp"

namespace CF {
namespace FVM {
namespace Core {

using namespace Common;

Common::ComponentBuilder < RoeCons1D, RiemannSolver, LibCore > RoeCons1D_Builder;

////////////////////////////////////////////////////////////////////////////////

RoeCons1D::RoeCons1D ( const std::string& name  )
: RiemannSolver(name),
  m_roe_avg(3)
{
  properties()["brief"] = std::string("Approximate Riemann solver Roe");
  properties()["description"] = std::string("Solves the Riemann problem using the Roe scheme");
}

////////////////////////////////////////////////////////////////////////////////

RoeCons1D::~RoeCons1D()
{
}

////////////////////////////////////////////////////////////////////////////////

void RoeCons1D::solve(const RealVector& left, const RealVector& right, const RealVector& normal ,
                            RealVector& interface_flux, Real& left_wave_speed, Real& right_wave_speed)
{
  // compute the roe average
  compute_roe_average(left,right,m_roe_avg);

  const Real r=m_roe_avg[0];
  const Real u=m_roe_avg[1]/r;
  const Real h = m_g*m_roe_avg[2]/r - 0.5*m_gm1*u*u;
  const Real a = sqrt(m_gm1*(h-u*u/2.));

  const Real p = a*a*r/m_g;
  CFinfo << "p = " << p << CFendl;
  CFinfo << "h = " << h << CFendl;


  const Real nx = normal[0];
  Real un = u*nx;

  // right eigenvectors
  RealMatrix3 right_eigenvectors; right_eigenvectors <<

        1.,           0.5*r/a,             0.5*r/a,
        u,            0.5*r/a*(u+a*nx),    0.5*r/a*(u-a*nx),
        0.5*u*u,      0.5*r/a*(h+u*a*nx),  0.5*r/a*(h-u*a*nx);

  // left eigenvectors = inverse(rc)
  RealMatrix3 left_eigenvectors; left_eigenvectors <<

        1.-0.5*m_gm1*u*u/(a*a),              m_gm1*u/(a*a),         -m_gm1/(a*a),
        a/r*(0.5*m_gm1*u*u/(a*a)-u*nx/a),    1./r*(nx-m_gm1*u/a),    m_gm1/(r*a),
        a/r*(0.5*m_gm1*u*u/(a*a)+u*nx/a),    -1./r*(nx+m_gm1*u/a),   m_gm1/(r*a);

  // eigenvalues
  RealVector3 eigenvalues(3); eigenvalues <<

        un,   un+a,   un-a;

//  CFinfo << "eigenvalues = \n" << eigenvalues << CFendl;

  // calculate absolute jacobian
  RealMatrix3 abs_jacobian = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;

//  CFinfo << "abs_jacobian = \n" << abs_jacobian << CFendl;
  // flux = central part + upwind part
  interface_flux = 0.5*(flux(left,normal)+flux(right,normal)) - 0.5*abs_jacobian*(right-left);

  left_wave_speed  =  un+a;
  right_wave_speed = -un+a;
}

////////////////////////////////////////////////////////////////////////////////

void RoeCons1D::compute_roe_average(const RealVector& left, const RealVector& right, RealVector& roe_avg) const
{
  const Real rho_L  = left[0];       const Real rho_R  = right[0];
  const Real rhou_L = left[1];       const Real rhou_R = right[1];
  const Real rhoE_L = left[2];       const Real rhoE_R = right[2];

  // convert to roe variables
  const Real u_L = rhou_L/rho_L;     const Real u_R = rhou_R/rho_R;
  const Real h_L = m_g*rhoE_L/rho_L - 0.5*m_gm1*u_L*u_L;
  const Real h_R = m_g*rhoE_R/rho_R - 0.5*m_gm1*u_R*u_R;

  const Real pL = m_gm1*(rhoE_L-0.5*rho_L*u_L*u_L);
  const Real pR = m_gm1*(rhoE_R-0.5*rho_R*u_R*u_R);
  CFinfo << "pL = " << pL << CFendl;
  CFinfo << "pR = " << pR << CFendl;

  const Real sqrt_rho_L = sqrt(rho_L);
  const Real sqrt_rho_R = sqrt(rho_R);

  // compute roe average quantities
  const Real rho_A = sqrt_rho_L * sqrt_rho_R;
  const Real u_A   = (sqrt_rho_L*u_L + sqrt_rho_R*u_R) / (sqrt_rho_L + sqrt_rho_R);
  const Real h_A   = (sqrt_rho_L*h_L + sqrt_rho_R*h_R) / (sqrt_rho_L + sqrt_rho_R);

  // return as conserved variables
  roe_avg[0] = rho_A;
  roe_avg[1] = rho_A * u_A;
  roe_avg[2] = rho_A/m_g * (h_A + 0.5*(m_gm1*u_A*u_A) );
}

////////////////////////////////////////////////////////////////////////////////

RealVector RoeCons1D::flux(const RealVector& state, const RealVector& normal) const
{
  const Real r=state[0];
  const Real u=state[1]/r;
  const Real rE = state[2];
  const Real p = m_gm1*(rE-0.5*r*u*u);
  const Real nx = normal[0];
  RealVector F(3);
  F <<     r*u*nx,   r*u*nx*u+p*nx,   (rE+p)*u*nx;

  return F;
}

////////////////////////////////////////////////////////////////////////////////

} // Core
} // FVM
} // CF
