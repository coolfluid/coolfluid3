// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
#include <iostream>
#include "cf3/physics/lineuler/lineuler2d/Functions.hpp"
#include "cf3/math/Defs.hpp"

namespace cf3 {
namespace physics {
namespace lineuler {
namespace lineuler2d {

//////////////////////////////////////////////////////////////////////////////////////////////

void compute_convective_flux( const Data& p, const ColVector_NDIM& normal,
                                    RowVector_NEQS& flux, Real& wave_speed )
{
  static Real u0n;
  static Real un;
  u0n = p.U0.dot(normal);
  un = p.U.dot(normal);

  flux[0] = u0n*p.cons[0] + p.rho0*un;
  flux[1] = u0n*p.cons[1] + p.p*normal[XX];
  flux[2] = u0n*p.cons[2] + p.p*normal[YY];
  flux[3] = u0n*p.cons[3] + p.rho0*un*p.c0*p.c0;
  
  wave_speed=std::abs(u0n)+p.c0;
}
    
void compute_convective_flux( const Data& p, const ColVector_NDIM& normal,
                              RowVector_NEQS& flux )
{
  static Real u0n;
  static Real un;
  u0n = p.U0.dot(normal);
  un = p.U.dot(normal);

  flux[0] = u0n*p.cons[0] + p.rho0*un;
  flux[1] = u0n*p.cons[1] + p.p*normal[XX];
  flux[2] = u0n*p.cons[2] + p.p*normal[YY];
  flux[3] = u0n*p.cons[3] + p.rho0*un*p.c0*p.c0;
}

void compute_convective_wave_speed( const Data& p, const ColVector_NDIM& normal,
                                    Real& wave_speed )
{
  wave_speed=std::abs(p.U0.dot(normal))+p.c0;
}

void compute_convective_eigenvalues( const Data& p, const ColVector_NDIM& normal,
                                     RowVector_NEQS& eigen_values )
{
  static Real u0n;
  u0n = p.U0.dot(normal);
  eigen_values << 
    u0n,
    u0n,
    u0n+p.c0,
    u0n-p.c0;
}

void compute_convective_right_eigenvectors( const Data& p, const ColVector_NDIM& normal,
                                            Matrix_NEQSxNEQS& right_eigenvectors )
{
  const Real  inv_c0  = 1./p.c0;
  const Real& nx = normal[XX];
  const Real& ny = normal[YY];

  right_eigenvectors <<
        1.,      0.,      0.5*inv_c0,   0.5*inv_c0,
        0.,      ny,      0.5*nx,      -0.5*nx,
        0.,     -nx,      0.5*ny,      -0.5*ny,
        0.,      0.,      0.5*p.c0,     0.5*p.c0;
}

void compute_convective_left_eigenvectors( const Data& p, const ColVector_NDIM& normal,
                                                 Matrix_NEQSxNEQS& left_eigenvectors )
{
  const Real  inv_c0 = 1./p.c0;
  const Real& nx = normal[XX];
  const Real& ny = normal[YY];

  // matrix of left eigenvectors (rows) = Rv.inverse()
  left_eigenvectors <<
        1.,      0.,      0.,          -inv_c0*inv_c0,
        0.,      ny,     -nx,           0,
        0.,      nx,      ny,           inv_c0,
        0.,     -nx,     -ny,           inv_c0;
}


void compute_rusanov_flux( const Data& left, const Data& right, const ColVector_NDIM& normal,
                           RowVector_NEQS& flux, Real& wave_speed )
{
  static RowVector_NEQS left_flux, right_flux;
  static Real left_wave_speed, right_wave_speed;
  compute_convective_flux( left,  normal, left_flux );
  compute_convective_flux( right, normal, right_flux);
  compute_convective_wave_speed( left, normal, wave_speed );
  flux  = 0.5*(left_flux+right_flux);
  flux -= 0.5*wave_speed*(right.cons - left.cons);
}

void compute_absolute_flux_jacobian( const Data& p, const ColVector_NDIM& normal,
                                     Matrix_NEQSxNEQS& absolute_flux_jacobian)
{
  const Real u0n = p.U0.dot(normal);
  const Real inv_2c  = 0.5/p.c0;
  const Real inv_2c2 = 0.5/(p.c0*p.c0);

  const Real& nx = normal[XX];
  const Real& ny = normal[YY];
  const Real nx2 = nx*nx;
  const Real ny2 = ny*ny;
  const Real absu0n = std::abs(u0n);
  const Real cpu = std::abs(p.c0+u0n);
  const Real cmu = std::abs(p.c0-u0n);
  const Real plus  = cmu + cpu;
  const Real minus = cpu - cmu;
  const Real pm2u  = plus - 2*absu0n;

  absolute_flux_jacobian <<
    absu0n,     (nx*minus)*inv_2c,              (ny*minus)*inv_2c,              pm2u*inv_2c2,
    0,          (2*ny2*absu0n + nx2*plus)*0.5,  (nx*ny*pm2u)*0.5,               (nx*minus)*inv_2c,
    0,          (nx*ny*pm2u)*0.5,               (2*nx2*absu0n + ny2*plus)*0.5,  (ny*minus)*inv_2c,
    0,          (p.c0*nx*minus)*0.5,            (p.c0*ny*minus)*0.5,            plus*0.5;
}

void compute_cir_flux( const Data& left, const Data& right, const ColVector_NDIM& normal,
                       RowVector_NEQS& flux, Real& wave_speed )
{
  static RowVector_NEQS flux_left, flux_right;
  compute_convective_flux(left, normal,flux_left);
  compute_convective_flux(right,normal,flux_right);

  // No Roe-average is needed as the eigen-system is only dependant of the mean flow
  static Matrix_NEQSxNEQS A;
  compute_absolute_flux_jacobian(left,normal,A);

  flux.noalias()  = 0.5*(flux_left+flux_right);
  flux.noalias() -= 0.5*A*(right.cons-left.cons).transpose();

  compute_convective_wave_speed(left,normal,wave_speed);
}

////////////////////////////////////////////////////////////////////////////////

void cons_to_char(const RowVector_NEQS& conservative,
                  const ColVector_NDIM& characteristic_normal,
                  const Real& c0,
                  RowVector_NEQS& characteristic)
{
  const Real& rho   = conservative[0];
  const Real& rho0u = conservative[1];
  const Real& rho0v = conservative[2];
  const Real& press = conservative[3];
  const Real& nx    = characteristic_normal[XX];
  const Real& ny    = characteristic_normal[YY];
  Real& S     = characteristic[0];
  Real& Omega = characteristic[1];
  Real& Aplus = characteristic[2];
  Real& Amin  = characteristic[3];

  S     =  rho - press/(c0*c0);
  Omega =  ny*rho0u - nx*rho0v;
  Aplus =  nx*rho0u + ny*rho0v + press/c0;
  Amin  = -nx*rho0u - ny*rho0v + press/c0;
}

////////////////////////////////////////////////////////////////////////////////

void char_to_cons(const RowVector_NEQS& characteristic, const ColVector_NDIM& characteristic_normal, const Real& c0, RowVector_NEQS& conservative)
{
  const Real& S     = characteristic[0];
  const Real& Omega = characteristic[1];
  const Real& Aplus = characteristic[2];
  const Real& Amin  = characteristic[3];
  const Real& nx    = characteristic_normal[XX];
  const Real& ny    = characteristic_normal[YY];
  Real& rho   = conservative[0];
  Real& rho0u = conservative[1];
  Real& rho0v = conservative[2];
  Real& press = conservative[3];

  const Real A     = Aplus+Amin;
  const Real omega = Aplus-Amin;

  rho   =  S + 0.5*A/c0;
  rho0u =  ny*Omega + 0.5*nx*omega;
  rho0v = -nx*Omega + 0.5*ny*omega;
  press =  0.5*c0*A;
}

//////////////////////////////////////////////////////////////////////////////////////////////

} // lineuler2d
} // lineuler
} // physics
} // cf3
