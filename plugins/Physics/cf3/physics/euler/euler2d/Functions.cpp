// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
#include <iostream>
#include "cf3/physics/euler/euler2d/Functions.hpp"
#include "cf3/math/Defs.hpp"

namespace cf3 {
namespace physics {
namespace euler {
namespace euler2d {

//////////////////////////////////////////////////////////////////////////////////////////////

void compute_convective_flux( const Data& p, const ColVector_NDIM& normal,
                                    RowVector_NEQS& flux, Real& wave_speed )
{
  static Real un;
  static Real rho_un;
  un = p.U.dot(normal);
  rho_un = p.rho * un;
  flux[0] = rho_un;
  flux[1] = rho_un * p.U[XX] + p.p * normal[XX];
  flux[2] = rho_un * p.U[YY] + p.p * normal[YY];
  flux[3] = rho_un * p.H;
  wave_speed=std::abs(un)+p.c;
}
    
void compute_convective_flux( const Data& p, const ColVector_NDIM& normal,
                                    RowVector_NEQS& flux )
{
  static Real un;
  static Real rho_un;
  un = p.U.dot(normal);
  rho_un = p.rho * un;
  flux[0] = rho_un;
  flux[1] = rho_un * p.U[XX] + p.p * normal[XX];
  flux[2] = rho_un * p.U[YY] + p.p * normal[YY];
  flux[3] = rho_un * p.H;
}

void compute_convective_wave_speed( const Data& p, const ColVector_NDIM& normal,
                                          Real& wave_speed )
{
  wave_speed=std::abs(p.U.dot(normal))+p.c;
}

void compute_convective_eigenvalues( const Data& p, const ColVector_NDIM& normal,
                                           RowVector_NEQS& eigen_values )
{
  static Real un;
  un = p.U.dot(normal);
  eigen_values <<
      un,
      un,
      un+p.c,
      un-p.c;
}

void compute_convective_right_eigenvectors( const Data& p, const ColVector_NDIM& normal,
                                                 Matrix_NEQSxNEQS& right_eigenvectors )
{
  const Real& u = p.U[XX];
  const Real& v = p.U[YY];
  const Real& nx = normal[XX];
  const Real& ny = normal[YY];
  const Real un = p.U.dot(normal);
  const Real us = u*ny - v*nx;
  right_eigenvectors <<
    1.,            0,           1,           1,
    u,             ny,          u+p.c*nx,    u-p.c*nx,
    v,            -nx,          v+p.c*ny,    v-p.c*ny,
    0.5*p.U2,      us,          p.H+p.c*un,  p.H-p.c*un;
}

void compute_convective_left_eigenvectors( const Data& p, const ColVector_NDIM& normal,
                                           Matrix_NEQSxNEQS& left_eigenvectors )
{
  const Real& u = p.U[XX];
  const Real& v = p.U[YY];
  const Real& nx = normal[XX];
  const Real& ny = normal[YY];
  const Real un = p.U.dot(normal);
  const Real us = u*ny - v*nx;
  const Real gm1 = p.gamma-1;
  const Real inv_c  = 1. / p.c;
  const Real inv_c2 = inv_c * inv_c;

  // matrix of left eigenvectors (rows) = Rv.inverse()
  left_eigenvectors <<
     2.-gm1*p.H*inv_c2,                  u*gm1*inv_c2,                 v*gm1*inv_c2,               -gm1*inv_c2,
    -us,                                 ny,                          -nx,                          0,
     0.5*inv_c2*(0.5*gm1*p.U2+p.c*un),  -0.5*inv_c2*(gm1*u-p.c*nx),   -0.5*inv_c2*(gm1*v-p.c*ny),   0.5*gm1*inv_c2,
     0.5*inv_c2*(0.5*gm1*p.U2-p.c*un),  -0.5*inv_c2*(gm1*u+p.c*nx),   -0.5*inv_c2*(gm1*v+p.c*ny),   0.5*gm1*inv_c2;
}


void compute_rusanov_flux( const Data& left, const Data& right, const ColVector_NDIM& normal,
                           RowVector_NEQS& flux, Real& wave_speed )
{
  static RowVector_NEQS left_flux, right_flux;
  static Real left_wave_speed, right_wave_speed;
  compute_convective_flux( left,  normal, left_flux,  left_wave_speed );
  compute_convective_flux( right, normal, right_flux, right_wave_speed);
  wave_speed = std::max(left_wave_speed,right_wave_speed);
  flux  = 0.5*(left_flux+right_flux);
  flux -= 0.5*wave_speed*(right.cons - left.cons);
}

void compute_roe_average( const Data& left, const Data& right,
                          Data& roe )
{
  static Real sqrt_rhoL;
  static Real sqrt_rhoR;
  sqrt_rhoL = std::sqrt(left.rho);
  sqrt_rhoR = std::sqrt(right.rho);
  roe.gamma = 0.5*(left.gamma+right.gamma);
  roe.rho   = sqrt_rhoL*sqrt_rhoR;
  roe.U     = (sqrt_rhoL*left.U + sqrt_rhoR*right.U) / (sqrt_rhoL + sqrt_rhoR);
  roe.H     = (sqrt_rhoL*left.H + sqrt_rhoR*right.H) / (sqrt_rhoL + sqrt_rhoR);
  roe.U2    = roe.U.squaredNorm();
  roe.c2    = (roe.gamma-1.)*(roe.H-0.5*roe.U2/roe.rho);
  roe.p     = roe.c2 * roe.rho / roe.gamma;
  roe.c     = std::sqrt(roe.c2);
}

void compute_roe_flux( const Data& left, const Data& right, const ColVector_NDIM& normal,
                       RowVector_NEQS& flux, Real& wave_speed )
{
  // Compute Roe average
  static Data roe;
  compute_roe_average(left,right,roe);

  // Compute the wave strengths dW
  static RowVector_NEQS dW;
  static ColVector_NDIM dU;
  static ColVector_NDIM s;
  static Real drho, dun, dus, dp;
  s << normal[YY], -normal[XX];
  drho = (right.rho - left.rho);
  dU   = (right.U   - left.U);
  dp   = (right.p   - left.p);
  dun  = dU.dot(normal);
  dus  = dU.dot(s);
  
  dW[0] = drho - dp/roe.c2;
  dW[1] = dus * roe.rho;
  dW[2] = 0.5*(dp/roe.c2 + dun*roe.rho/roe.c);
  dW[3] = 0.5*(dp/roe.c2 - dun*roe.rho/roe.c);

  // Compute the wave speeds
  static RowVector_NEQS lambda;
  compute_convective_eigenvalues(roe, normal, lambda);

  static Matrix_NEQSxNEQS R;
  compute_convective_right_eigenvectors(roe, normal, R);

  static RowVector_NEQS flux_left, flux_right;
  compute_convective_flux(left,normal,flux_left);
  compute_convective_flux(right,normal,flux_right);
  flux.noalias() = 0.5*(flux_left+flux_right);
  for (Uint k=0; k<NEQS; ++k)
  {
    for (Uint eq=0; eq<NEQS; ++eq)
      flux[eq] -= 0.5*std::abs(lambda[k]) * dW[k] * R(eq,k);
  }

  compute_convective_wave_speed(roe, normal, wave_speed);
}

void compute_hlle_flux( const Data& left, const Data& right, const ColVector_NDIM& normal,
                        RowVector_NEQS& flux, Real& wave_speed )
{
  // Compute Roe average
  static Data roe;
  compute_roe_average(left,right,roe);

  static RowVector_NEQS lambda_left, lambda_right, lambda_roe;
  compute_convective_eigenvalues(left,  normal, lambda_left);
  compute_convective_eigenvalues(right, normal, lambda_right);
  compute_convective_eigenvalues(roe,   normal, lambda_roe);

  static Real wave_speed_left, wave_speed_right;
  wave_speed_left  = std::min(lambda_left.minCoeff(),  lambda_roe.minCoeff()); // u - c
  wave_speed_right = std::max(lambda_right.maxCoeff(), lambda_roe.maxCoeff()); // u + c

  if (wave_speed_left >= 0.) // supersonic to the right
  {
    compute_convective_flux(left,normal,flux);
  }
  else if (wave_speed_right <= 0.) // supersonic to the left
  {
    compute_convective_flux(right,normal,flux);
  }
  else // intermediate state
  {
    static RowVector_NEQS flux_left, flux_right;
    compute_convective_flux(left,  normal, flux_left );
    compute_convective_flux(right, normal, flux_right);
    for (Uint eq=0; eq<NEQS; ++eq)
    {
      flux[eq] =  (wave_speed_right*flux_left[eq]-wave_speed_left*flux_right[eq]);
      flux[eq] += (wave_speed_left*wave_speed_right)*(right.cons[eq]-left.cons[eq]);
      flux[eq] /= (wave_speed_right-wave_speed_left);
    }
  }
  compute_convective_wave_speed(roe,normal,wave_speed);
}
//////////////////////////////////////////////////////////////////////////////////////////////

} // euler2d
} // euler
} // physics
} // cf3
