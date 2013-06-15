// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "cf3/physics/navierstokes/navierstokes1d/Functions.hpp"
#include "cf3/math/Defs.hpp"
#include "cf3/common/BasicExceptions.hpp"

namespace cf3 {
namespace physics {
namespace navierstokes {
namespace navierstokes1d {

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

  // Viscous stress tensor
  // tau_ij = mu ( du_i/dx_j + du_j/dx_i - delta_ij 2/3 div(u) )
  Real tau_xx = p.mu*4./3.*p.grad_u[XX];

  // Heat flux
  Real heat_flux = -p.kappa*(p.grad_T[XX]*nx);

  flux[0] = 0.;
  flux[1] = tau_xx*nx;
  flux[2] = (tau_xx*p.u)*nx - heat_flux;
}

void compute_diffusive_wave_speed( const Data& p, const ColVector_NDIM& normal,
                                   Real& wave_speed )
{
  // maximum of kinematic viscosity nu and thermal diffusivity alpha
  wave_speed = std::max(p.mu/p.rho, p.kappa/(p.rho*p.Cp));
}

//////////////////////////////////////////////////////////////////////////////////////////////

} // navierstokes1d
} // navierstokes
} // physics
} // cf3
