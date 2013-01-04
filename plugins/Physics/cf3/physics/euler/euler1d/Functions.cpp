// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
#include <iostream>
#include "cf3/physics/euler/euler1d/Functions.hpp"
#include "cf3/math/Defs.hpp"
#include "cf3/math/Consts.hpp"

namespace cf3 {
namespace physics {
namespace euler {
namespace euler1d {

//////////////////////////////////////////////////////////////////////////////////////////////

void compute_convective_flux( const Data& p, const ColVector_NDIM& normal,
                                    RowVector_NEQS& flux, Real& wave_speed )
{
  static Real un;
  static Real rho_un;
  un = p.u * normal[XX];
  rho_un = p.rho * un;
  flux[0] = rho_un;
  flux[1] = rho_un * p.u + p.p * normal[XX];
  flux[2] = rho_un * p.H;
  wave_speed=std::abs(un)+p.c*std::abs(normal[XX]);
}
    
void compute_convective_flux( const Data& p, const ColVector_NDIM& normal,
                                    RowVector_NEQS& flux )
{
  static Real un;
  static Real rho_un;
  un = p.u * normal[XX];
  rho_un = p.rho * un;
  flux[0] = rho_un;
  flux[1] = rho_un * p.u + p.p *normal[XX];
  flux[2] = rho_un * p.H;
}

void compute_convective_wave_speed( const Data& p, const ColVector_NDIM& normal,
                                          Real& wave_speed )
{
  wave_speed=std::abs(p.u * normal[XX])+p.c*std::abs(normal[XX]);
}

void compute_convective_eigenvalues( const Data& p, const ColVector_NDIM& normal,
                                           RowVector_NEQS& eigen_values )
{
  static Real un;
  static Real cn;
  un = p.u * normal[XX];
  cn = p.c * normal[XX];
  eigen_values[0] = un;
  eigen_values[1] = un+cn;
  eigen_values[2] = un-cn;
}

void compute_convective_right_eigenvectors( const Data& p, const ColVector_NDIM& normal,
                                                 Matrix_NEQSxNEQS& right_eigenvectors )
{
  right_eigenvectors <<
    1.,            1,           1,
    p.u,           p.u+p.c,     p.u-p.c,
    0.5*p.u*p.u,   p.H+p.c*p.u, p.H-p.c*p.u;
}

void compute_convective_left_eigenvectors( const Data& p, const ColVector_NDIM& normal,
                                                 Matrix_NEQSxNEQS& left_eigenvectors )
{
  const Real nx = normal[XX];
  const Real gm1 = p.gamma-1;
  const Real inv_c  = 1. / p.c;
  const Real inv_c2 = inv_c * inv_c;
  const Real M = p.u*inv_c;
  const Real M2 = M*M;

  // matrix of left eigenvectors (rows) = Rv.inverse()
  left_eigenvectors <<
     1.-0.5*gm1*M2,            p.u*gm1*inv_c2,              -gm1*inv_c2,
     0.25*gm1*M2-0.5*M,       -0.5*inv_c2*(gm1*p.u-p.c),     0.5*gm1*inv_c2,
     0.25*gm1*M2+0.5*M,       -0.5*inv_c2*(gm1*p.u+p.c),     0.5*gm1*inv_c2;
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
  // Iterative solvers could temporarily create negative density.
  // Clip it to zero only in computation of the Roe average
  sqrt_rhoL = std::sqrt(std::abs(left.rho));//,math::Consts::eps()));
  sqrt_rhoR = std::sqrt(std::abs(right.rho));//,math::Consts::eps()));
  roe.gamma = 0.5*(left.gamma+right.gamma);
  roe.rho   = sqrt_rhoL*sqrt_rhoR;
  roe.u     = (sqrt_rhoL*left.u + sqrt_rhoR*right.u) / (sqrt_rhoL + sqrt_rhoR);
  roe.H     = (sqrt_rhoL*std::abs(left.H) + sqrt_rhoR*std::abs(right.H)) / (sqrt_rhoL + sqrt_rhoR);
  roe.c2    = (roe.gamma-1.)*(roe.H-0.5*roe.u*roe.u/roe.rho);
  roe.p     = roe.c2 * roe.rho / roe.gamma;
  roe.c     = std::sqrt(roe.c2);
}

void compute_roe_flux( const Data& left, const Data& right, const ColVector_NDIM& normal,
                       RowVector_NEQS& flux, Real& wave_speed )
{
  if (left.rho<0 || right.rho<0)
  {
    throw common::BadValue(FromHere(), "negative density");
  }
  // Compute Roe average
  static Data roe;
  compute_roe_average(left,right,roe);

//  std::cout << "roe.rho = " << roe.rho << std::endl;
//  std::cout << "roe.u = " << roe.u << std::endl;
//  std::cout << "roe.H = " << roe.H << std::endl;
//  std::cout << "roe.c2 = " << roe.c2 << std::endl;
//  std::cout << "roe.p = " << roe.p << std::endl;
//  std::cout << "roe.c = " << roe.c << std::endl;

  // Compute the wave strengths dW
  static RowVector_NEQS dW;
  Real drho = (right.rho - left.rho);
  Real du   = (right.u   - left.u);
  Real dp   = (right.p   - left.p);
  dW[0] = drho - dp/roe.c2;
  dW[1] = 0.5*(dp/roe.c2 + du*roe.rho/roe.c);
  dW[2] = 0.5*(dp/roe.c2 - du*roe.rho/roe.c);

//  std::cout << "dW = " << dW << std::endl;

  // Compute the wave speeds
  static RowVector_NEQS lambda;
  compute_convective_eigenvalues(roe, normal, lambda);

//  std::cout << "lambda = " << lambda << std::endl;
  static Matrix_NEQSxNEQS R;
  compute_convective_right_eigenvectors(roe, normal, R);

//  std::cout << "R = " << R << std::endl;

  static RowVector_NEQS flux_left, flux_right;
  compute_convective_flux(left,normal,flux_left);
  compute_convective_flux(right,normal,flux_right);
//  std::cout << "FL = " << flux_left << std::endl;
//  std::cout << "FR = " << flux_right << std::endl;
  flux.noalias() = 0.5*(flux_left+flux_right);
  for (Uint k=0; k<NEQS; ++k)
  {
    for (Uint eq=0; eq<NEQS; ++eq)
      flux[eq] -= 0.5*std::abs(lambda[k]) * dW[k] * R(eq,k);
  }

  compute_convective_wave_speed(roe, normal, wave_speed);

  // Previous calculation should be equivalent but faster than:
  //      static Matrix_NEQSxNEQS R;
  //      compute_convective_right_eigenvectors(roe, normal, R);
  //      static Matrix_NEQSxNEQS L;
  //      compute_convective_left_eigenvectors(roe, normal, L);
  //      RowVector_NEQS upwind = 0.5*(R*lambda.cwiseAbs().asDiagonal()*L*((right.cons-left.cons).transpose()));
  //      flux = 0.5*(flux_left+flux_right)-upwind;
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

} // euler1D
} // euler
} // physics
} // cf3
