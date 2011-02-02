// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/Log.hpp"
#include "Common/Foreach.hpp"

#include "FVM/RoeFluxSplitter.hpp"

namespace CF {
namespace FVM {

using namespace Common;

Common::ComponentBuilder < RoeFluxSplitter, Component, LibFVM > RoeFluxSplitter_Builder;

////////////////////////////////////////////////////////////////////////////////

RoeFluxSplitter::RoeFluxSplitter ( const std::string& name  ) 
: Component(name),
  m_g(1.4),
  m_gm1(m_g-1.)
{
  properties()["brief"] = std::string("Roe Flux Splitter");
  properties()["description"] = std::string("Solves the Riemann problem using the Roe scheme");
}

////////////////////////////////////////////////////////////////////////////////

RoeFluxSplitter::~RoeFluxSplitter()
{
}

////////////////////////////////////////////////////////////////////////////////

RealVector RoeFluxSplitter::solve(const RealVector& left, const RealVector& right) const
{
  // compute the roe average
  RealVector rho_avg = roe_average(left,right);

  const Real r=rho_avg[0];
  const Real u=rho_avg[1]/r;
  const Real h = m_g*rho_avg[2]/r - 0.5*m_gm1*u*u;
  const Real a = sqrt(m_gm1*(h-u*u/2.));

  // Compute eigenvectors (rc) and eigenvalues (lambda)
  // dFdU = lc(3x3) . Lambda(3,3) . rc(3,3)   (lc = inv(rc))
  RealMatrix rc(3,3); rc << 
        -r/(2.*a),   -r/(2.*a)*(u-a),   -r/(2.*a)*(h-a*u),
        1.,          u,                 0.5*u*u,
        r/(2.*a),    r/(2.*a)*(u+a),    r/(2.*a)*(h+a*u);

  RealMatrix lc(3,3); lc <<  
         m_gm1/(r*a)*(-u*u/2.0-a*u/m_gm1),        m_gm1/(r*a)*(u+a/m_gm1),   -m_gm1/(r*a),
         m_gm1/(r*a)*(r/a*(-u*u/2.0+a*a/m_gm1)),  m_gm1/(r*a)*(r/a*u),       m_gm1/(r*a)*(-r/a),
         m_gm1/(r*a)*(u*u/2.0 - a*u/m_gm1),       m_gm1/(r*a)*(-u+a/m_gm1),  m_gm1/(r*a);

  RealVector lambda(3); lambda <<    u-a,   u,   u+a;
  
  RealVector alpha = lc * (right-left);
  RealVector upwind_part(3);
  for(Uint j=0; j<3; j++)
  {
    upwind_part[j]=0;
    for(Uint i=0; i<3; i++)
      upwind_part[j] += std::abs(lambda[i]) * alpha[i] * rc(i,j);
  }
  return 0.5*(flux(left)+flux(right))-0.5*upwind_part;
}

////////////////////////////////////////////////////////////////////////////////

RealVector RoeFluxSplitter::roe_average(const RealVector& left, const RealVector& right) const
{  
  const Real rho_L  = left[0];       const Real rho_R  = right[0];
  const Real rhou_L = left[1];       const Real rhou_R = right[1];
  const Real rhoE_L = left[2];       const Real rhoE_R = right[2];
  
  // convert to roe variables
  const Real u_L = rhou_L/rho_L;     const Real u_R = rhou_R/rho_R;
  const Real h_L = m_g*rhoE_L/rho_L - 0.5*m_gm1*u_L*u_L;
  const Real h_R = m_g*rhoE_R/rho_R - 0.5*m_gm1*u_R*u_R;
  
  const Real sqrt_rho_L = sqrt(rho_L);
  const Real sqrt_rho_R = sqrt(rho_R);

  // compute roe average quantities
  const Real rho_A = sqrt_rho_L * sqrt_rho_R;
  const Real u_A   = (sqrt_rho_L*u_L + sqrt_rho_R*u_R) / (sqrt_rho_L + sqrt_rho_R);
  const Real h_A   = (sqrt_rho_L*h_L + sqrt_rho_R*h_R) / (sqrt_rho_L + sqrt_rho_R);
  
  // return as conserved variables
  RealVector roe_avg(3);
  roe_avg[0] = rho_A;
  roe_avg[1] = rho_A * u_A;
  roe_avg[2] = rho_A/m_g * (h_A + 0.5*(m_gm1*u_A*u_A) );
  return roe_avg;
}

////////////////////////////////////////////////////////////////////////////////

RealVector RoeFluxSplitter::flux(const RealVector& state) const
{
  const Real r=state[0];
  const Real u=state[1]/r;
  const Real rE = state[2];
  const Real p = m_gm1*(rE-0.5*r*u*u);
  RealVector F(3);
  F <<     r*u,   r*u*u+p,   (rE+p)*u;
  return F;

}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF
