// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "FVM/RoeCons2D.hpp"

namespace CF {
namespace FVM {

using namespace Common;

Common::ComponentBuilder < RoeCons2D, RiemannSolver, LibFVM > RoeCons2D_Builder;

////////////////////////////////////////////////////////////////////////////////

RoeCons2D::RoeCons2D ( const std::string& name  ) 
: RiemannSolver(name),
  m_roe_avg(4)
{
  properties()["brief"] = std::string("Roe Flux Splitter");
  properties()["description"] = std::string("Solves the Riemann problem using the Roe scheme");
}

////////////////////////////////////////////////////////////////////////////////

RoeCons2D::~RoeCons2D()
{
}

////////////////////////////////////////////////////////////////////////////////

void RoeCons2D::solve(const RealVector& left, const RealVector& right, const RealVector& normal , 
                            RealVector& interface_flux, Real& left_wave_speed, Real& right_wave_speed)
{
  // compute the roe average
  compute_roe_average(left,right,m_roe_avg);

  const Real r=m_roe_avg[0];
  const Real u=m_roe_avg[1]/r;
  const Real v=m_roe_avg[2]/r;
  const Real h = m_g*m_roe_avg[3]/r - 0.5*m_gm1*u*u;
  const Real a = sqrt(m_gm1*(h-0.5*(u*u+v*v)));

  const Real nx = normal[0];
  const Real ny = normal[1];
  const Real un = u*nx + v*ny; // normal roe-average speed 
  
  // right eigenvectors
  right_eigenvectors << 
  
        1.,             0.,             0.5*r/a,             0.5*r/a,
        u,              r*ny,           0.5*r/a*(u+a*nx),    0.5*r/a*(u-a*nx),
        v,              -r*nx,          0.5*r/a*(v+a*ny),    0.5*r/a*(v-a*ny),
        0.5*(u*u+v*v),  r*(u*ny-v*nx),  0.5*r/a*(h+a*un),    0.5*r/a*(h-a*un);


  // left eigenvectors = inverse(rc)
  left_eigenvectors << 
  
        1.-0.5*m_gm1*(u*u+v*v)/(a*a),            m_gm1*u/(a*a),           m_gm1*v/(a*a),        -m_gm1/(a*a),
        1./r*(v*nx-u*ny),                        1./r*ny,                -1./r*nx,               0.,
        a/r*(0.5*m_gm1*(u*u+v*v)/(a*a)-un/a),    1./r*(nx-m_gm1*u/a),     1./r*(ny-m_gm1*v/a),   m_gm1/(r*a),
        a/r*(0.5*m_gm1*(u*u+v*v)/(a*a)+un/a),   -1./r*(nx+m_gm1*u/a),    -1./r*(ny+m_gm1*v/a),   m_gm1/(r*a);
  

  // eigenvalues
  eigenvalues <<    
  
        un, un,  un+a,  un-a;

  
  // calculate absolute jacobian
  abs_jacobian = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;

  compute_flux(left,normal,F_L);
  compute_flux(right,normal,F_R);
  // flux = central part + upwind part
  interface_flux = 0.5*(F_L + F_R) - 0.5*abs_jacobian*(right-left);

  left_wave_speed  =  un+a;
  right_wave_speed = -un+a;
}

////////////////////////////////////////////////////////////////////////////////

void RoeCons2D::compute_roe_average(const RealVector& left, const RealVector& right, RealVector& roe_avg) const
{  
  const Real rho_L  = left[0];       const Real rho_R  = right[0];
  const Real rhou_L = left[1];       const Real rhou_R = right[1];
  const Real rhov_L = left[2];       const Real rhov_R = right[2];
  const Real rhoE_L = left[3];       const Real rhoE_R = right[3];
  
  // convert to roe variables
  const Real u_L = rhou_L/rho_L;     const Real u_R = rhou_R/rho_R;
  const Real v_L = rhov_L/rho_L;     const Real v_R = rhov_R/rho_R;
  const Real h_L = m_g*rhoE_L/rho_L - 0.5*m_gm1*(u_L*u_L+v_L*v_L);
  const Real h_R = m_g*rhoE_R/rho_R - 0.5*m_gm1*(u_R*u_R+v_R*v_R);
  
  const Real sqrt_rho_L = sqrt(rho_L);
  const Real sqrt_rho_R = sqrt(rho_R);

  // compute roe average quantities
  const Real rho_A = sqrt_rho_L * sqrt_rho_R;
  const Real u_A   = (sqrt_rho_L*u_L + sqrt_rho_R*u_R) / (sqrt_rho_L + sqrt_rho_R);
  const Real v_A   = (sqrt_rho_L*v_L + sqrt_rho_R*v_R) / (sqrt_rho_L + sqrt_rho_R);
  const Real h_A   = (sqrt_rho_L*h_L + sqrt_rho_R*h_R) / (sqrt_rho_L + sqrt_rho_R);
  
  // return as conserved variables
  roe_avg[0] = rho_A;
  roe_avg[1] = rho_A * u_A;
  roe_avg[2] = rho_A * v_A;
  roe_avg[3] = rho_A/m_g * (h_A + 0.5*(m_gm1*(u_A*u_A+v_A*v_A) ) );

}

////////////////////////////////////////////////////////////////////////////////

RealVector RoeCons2D::flux(const RealVector& state, const RealVector& normal) const
{
  const Real r=state[0];
  const Real u=state[1]/r;
  const Real v=state[2]/r;
  const Real rE = state[3];
  const Real p = m_gm1*(rE-0.5*r*(u*u+v*v));
  const Real un = u*normal[0] + v*normal[1]; // normal speed
  RealVector4 F;
  F <<     r*un,   
           r*u*un + p*normal[0],
           r*v*un + p*normal[1],
           (rE+p)*un;
  return F;
}

////////////////////////////////////////////////////////////////////////////////

void RoeCons2D::compute_flux(const RealVector& state, const RealVector& normal, RealVector4& flux) const
{
  const Real r=state[0];
  const Real u=state[1]/r;
  const Real v=state[2]/r;
  const Real rE = state[3];
  const Real p = m_gm1*(rE-0.5*r*(u*u+v*v));
  const Real un = u*normal[0] + v*normal[1]; // normal speed
  flux <<  r*un,   
           r*u*un + p*normal[0],
           r*v*un + p*normal[1],
           (rE+p)*un;
}
////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF
