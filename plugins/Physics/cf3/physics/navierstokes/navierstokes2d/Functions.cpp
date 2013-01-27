// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "cf3/physics/navierstokes/navierstokes2d/Functions.hpp"
#include "cf3/math/Defs.hpp"
#include "cf3/common/BasicExceptions.hpp"

namespace cf3 {
namespace physics {
namespace navierstokes {
namespace navierstokes2d {

//////////////////////////////////////////////////////////////////////////////////////////////

void compute_diffusive_flux( const Data& p, const ColVector_NDIM& normal,
                             RowVector_NEQS& flux, Real& wave_speed )
{
  compute_diffusive_flux(p,normal,flux);
  compute_diffusive_wave_speed(p,normal,wave_speed);
}
    
void compute_diffusive_flux( const Data& p, const ColVector_NDIM& normal,
                             RowVector_NEQS& flux )
{
  const Real& nx = normal[XX];
  const Real& ny = normal[YY];

  Real two_third_divergence_U = 2./3.*(p.grad_u[XX] + p.grad_v[YY]);

  // Viscous stress tensor
  // tau_ij = mu ( du_i/dx_j + du_j/dx_i - delta_ij 2/3 div(u) )
  Real tau_xx = p.mu*(2.*p.grad_u[XX] - two_third_divergence_U);
  Real tau_yy = p.mu*(2.*p.grad_v[YY] - two_third_divergence_U);
  Real tau_xy = p.mu*(p.grad_u[YY] + p.grad_v[XX]);

  // Heat flux
  Real heat_flux = -p.k*(p.grad_T[XX]*nx + p.grad_T[YY]*ny);

  flux[0] = 0.;
  flux[1] = tau_xx*nx + tau_xy*ny;
  flux[2] = tau_xy*nx + tau_yy*ny;
  flux[3] = (tau_xx*p.U[XX] + tau_xy*p.U[YY])*nx + (tau_xy*p.U[XX] + tau_yy*p.U[YY])*ny - heat_flux;
}

void compute_diffusive_wave_speed( const Data& p, const ColVector_NDIM& normal,
                                   Real& wave_speed )
{
  // maximum of kinematic viscosity nu and thermal diffusivity alpha
  wave_speed = std::max(p.mu/p.rho, p.k/(p.rho*p.Cp));
}

//////////////////////////////////////////////////////////////////////////////////////////////

} // navierstokes2d
} // navierstokes
} // physics
} // cf3
