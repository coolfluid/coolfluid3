// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#ifndef cf3_physics_NavierStokes_Cons3D_hpp
#define cf3_physics_NavierStokes_Cons3D_hpp

#include <iostream>

#include "common/StringConversion.hpp"
#include "common/BasicExceptions.hpp"

#include "math/Defs.hpp"

#include "physics/Variables.hpp"

#include "NavierStokes3D.hpp"

namespace cf3 {
namespace physics {
namespace NavierStokes {

///////////////////////////////////////////////////////////////////////////////////////

class NavierStokes_API Cons3D : public VariablesT<Cons3D> {

public: //typedefs

  typedef NavierStokes3D     MODEL;

  enum { Rho = 0, RhoU = 1, RhoV = 2, RhoW = 3, RhoE = 4 };

  
  

public: // functions

  /// constructor
  /// @param name of the component
  Cons3D ( const std::string& name );

  /// virtual destructor
  virtual ~Cons3D();

  /// Get the class name
  static std::string type_name () { return "Cons3D"; }

  /// compute physical properties
  template < typename CV, typename SV, typename GM >
  static void compute_properties ( const CV& coord,
                                   const SV& sol,
                                   const GM& grad_vars,
                                   MODEL::Properties& p )
  {
    p.coords    = coord;       // cache the coordiantes locally
    p.vars      = sol;         // cache the variables locally
    p.grad_vars = grad_vars;   // cache the gradient of variables locally

    p.rho   = sol[Rho ];
    p.rhou  = sol[RhoU];
    p.rhov  = sol[RhoV];
    p.rhow  = sol[RhoW];
    p.rhoE  = sol[RhoE];

    p.inv_rho = 1. / p.rho;

    p.u   = p.rhou * p.inv_rho;
    p.v   = p.rhov * p.inv_rho;
    p.w   = p.rhow * p.inv_rho;

    p.uuvvww = p.u*p.u + p.v*p.v + p.w*p.w;

    p.P = p.gamma_minus_1 * ( p.rhoE - 0.5 * p.rho * p.uuvvww );

    if( p.P <= 0. )
    {
          std::cout << "rho    : " << p.rho    << std::endl;
          std::cout << "rhou   : " << p.rhou   << std::endl;
          std::cout << "rhov   : " << p.rhov   << std::endl;
          std::cout << "rhow   : " << p.rhow   << std::endl;
          std::cout << "rhoE   : " << p.rhoE   << std::endl;
          std::cout << "P      : " << p.P      << std::endl;
          std::cout << "u      : " << p.u      << std::endl;
          std::cout << "v      : " << p.v      << std::endl;
          std::cout << "uuvvww : " << p.uuvvww << std::endl;


      throw common::BadValue( FromHere(), "Pressure is negative at coordinates ["
                                   + common::to_str(coord[XX]) + ","
                                   + common::to_str(coord[YY]) + ","
                                   + common::to_str(coord[ZZ]) + "]");
    }

    const Real RT = p.P * p.inv_rho;    // RT = p/rho

    p.E = p.rhoE * p.inv_rho;           // E = rhoE / rho

    p.H = p.E + RT;                     // H = E + p/rho

    p.a = sqrt( p.gamma * RT );

    p.a2 = p.a * p.a;

    p.Ma = sqrt( p.uuvvww / p.a2 );

    p.T = RT / p.R;

    p.half_gm1_v2 = 0.5 * p.gamma_minus_1 * p.uuvvww;
  }

  template < typename VectorT >
  static void compute_variables ( const MODEL::Properties& p, VectorT& vars )
  {
    vars[Rho]  = p.rho;
    vars[RhoU] = p.rhou;
    vars[RhoV] = p.rhov;
    vars[RhoW] = p.rhow;
    vars[RhoE] = p.rhoE;
  }

  /// compute the physical flux
  template < typename FM >
  static void flux( const MODEL::Properties& p,
                    FM& flux)
  {
    flux(0,XX) = p.rhou;                       // rho.u
    flux(1,XX) = p.rhou * p.u + p.P;           // rho.u^2 + P
    flux(2,XX) = p.rhou * p.rhov * p.inv_rho;  // rho.u.v
    flux(3,XX) = p.rhou * p.rhow * p.inv_rho;  // rho.u.w
    flux(4,XX) = p.rhou * p.H;                 // rho.u.H

    flux(0,YY) = p.rhov;                       // rho.v
    flux(1,YY) = p.rhov * p.rhou * p.inv_rho;  // rho.v.u
    flux(2,YY) = p.rhov * p.v + p.P;           // rho.v^2 + P
    flux(3,YY) = p.rhov * p.rhow * p.inv_rho;  // rho.v.w
    flux(4,YY) = p.rhov * p.H;                 // rho.v.H

    flux(0,ZZ) = p.rhow;                       // rho.w
    flux(1,ZZ) = p.rhow * p.rhou * p.inv_rho;  // rho.w.u
    flux(2,ZZ) = p.rhow * p.rhov * p.inv_rho;  // rho.w.v
    flux(3,ZZ) = p.rhow * p.w + p.P;           // rho.w^2 + P
    flux(4,ZZ) = p.rhow * p.H;                 // rho.w.H
  }

  /// compute the physical flux
  template < typename FM , typename GV>
  static void flux( const MODEL::Properties& p,
                    const GV& direction,
                    FM& flux)
  {
    const Real rhoum = p.rhou * direction[XX]
                     + p.rhov * direction[YY]
                     + p.rhow * direction[ZZ];

    flux[0] = rhoum;
    flux[1] = rhoum * p.u + p.P*direction[XX];
    flux[2] = rhoum * p.v + p.P*direction[YY];
    flux[3] = rhoum * p.w + p.P*direction[ZZ];
    flux[4] = rhoum * p.H;
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv)
  {
    const Real um = p.u * direction[XX]
                  + p.v * direction[YY]
                  + p.w * direction[ZZ];

    Dv[0] = um;
    Dv[1] = um;
    Dv[2] = um;
    Dv[3] = um + p.a;
    Dv[4] = um - p.a;
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV, typename OP >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv,
                                         OP& op )

  {
    const Real um = p.u * direction[XX]
                  + p.v * direction[YY]
                  + p.w * direction[ZZ];

    const Real op_um = op(um);

    Dv[0] = op_um;
    Dv[1] = op_um;
    Dv[2] = op_um;
    Dv[3] = op_um + p.a;
    Dv[4] = op_um - p.a;
  }

  /// decompose the eigen structure of the flux jacobians projected on the gradients
  template < typename GV, typename EM, typename EV >
  static void flux_jacobian_eigen_structure(const MODEL::Properties& p,
                                            const GV& direction,
                                            EM& Rv,
                                            EM& Lv,
                                            EV& Dv)
  {
    const Real nx = direction[XX];
    const Real ny = direction[YY];
    const Real nz = direction[ZZ];

    const Real inv_a  = 1. / p.a;
    const Real inv_a2 = inv_a * inv_a;

    const Real um = p.u * nx + p.v * ny;
    const Real ra = 0.5 * p.rho * inv_a;

    const Real gu_a = p.gamma_minus_1 * p.u * inv_a;
    const Real gv_a = p.gamma_minus_1 * p.v * inv_a;
    const Real gw_a = p.gamma_minus_1 * p.w * inv_a;
    const Real rho_a = p.rho * p.a;

    const Real gm1_ov_rhoa = p.gamma_minus_1 / rho_a;

    const Real ke  = 0.5 * p.uuvvww;
    const Real gm2 = p.half_gm1_v2 * inv_a2;
    const Real k2  = 1.0 - gm2;
    const Real k3  = - p.gamma_minus_1 * inv_a2;

    // matrix of right eigen vectors R

    Rv(0,0) = nx;
    Rv(0,1) = ny;
    Rv(0,2) = nz;
    Rv(0,3) = ra;
    Rv(0,4) = ra;

    Rv(1,0) = p.u*nx;
    Rv(1,1) = p.u*ny - p.rho*nz;
    Rv(1,2) = p.u*nz + p.rho*ny;
    Rv(1,3) = ra*(p.u + p.a*nx);
    Rv(1,4) = ra*(p.u - p.a*nx);

    Rv(2,0) = p.v*nx + p.rho*nz;
    Rv(2,1) = p.v*ny;
    Rv(2,2) = p.v*nz - p.rho*nx;
    Rv(2,3) = ra*(p.v + p.a*ny);
    Rv(2,4) = ra*(p.v - p.a*ny);

    Rv(3,0) = p.w*nx - p.rho*ny;
    Rv(3,1) = p.w*ny + p.rho*nx;
    Rv(3,2) = p.w*nz;
    Rv(3,3) = ra*(p.w + p.a*nz);
    Rv(3,4) = ra*(p.w - p.a*nz);

    Rv(4,0) = ke*nx + p.rho*(p.v*nz - p.w*ny);
    Rv(4,1) = ke*ny + p.rho*(p.w*nx - p.u*nz);
    Rv(4,2) = ke*nz + p.rho*(p.u*ny - p.v*nx);
    Rv(4,3) = ra*(p.H + p.a*um);
    Rv(4,4) = ra*(p.H - p.a*um);

    // matrix of left eigen vectors L = R.inverse();

    Lv(0,0) = nx*k2 - p.inv_rho*(p.v*nz - p.w*ny);
    Lv(0,1) = gu_a*nx;
    Lv(0,2) = gv_a*nx + nz*p.inv_rho;
    Lv(0,3) = gw_a*nx - ny*p.inv_rho;
    Lv(0,4) = k3*nx;

    Lv(1,0) = ny*k2 - p.inv_rho*(p.w*nx - p.u*nz);
    Lv(1,1) = gu_a*ny - nz*p.inv_rho;
    Lv(1,2) = gv_a*ny;
    Lv(1,3) = gw_a*ny + nx*p.inv_rho;
    Lv(1,4) = k3*ny;

    Lv(2,0) = nz*k2 - p.inv_rho*(p.u*ny - p.v*nx);
    Lv(2,1) = gu_a*nz + ny*p.inv_rho;
    Lv(2,2) = gv_a*nz - nx*p.inv_rho;
    Lv(2,3) = gw_a*nz;
    Lv(2,4) = k3*nz;

    Lv(3,0) = p.a*p.inv_rho*(gm2 - um/p.a);
    Lv(3,1) = p.inv_rho*(nx - gu_a);
    Lv(3,2) = p.inv_rho*(ny - gv_a);
    Lv(3,3) = p.inv_rho*(nz - gw_a);
    Lv(3,4) = gm1_ov_rhoa;

    Lv(4,0) = p.a*p.inv_rho*(gm2 + um/p.a);
    Lv(4,1) = p.inv_rho*(-nx - gu_a);
    Lv(4,2) = p.inv_rho*(-ny - gv_a);
    Lv(4,3) = p.inv_rho*(-nz - gw_a);
    Lv(4,4) = gm1_ov_rhoa;

    // diagonal matrix of eigen values

    Dv[0] = um;
    Dv[1] = um;
    Dv[2] = um;
    Dv[3] = um + p.a;
    Dv[4] = um - p.a;
  }

  /// compute the PDE residual
  template < typename JM, typename RV >
  static void residual(const MODEL::Properties& p,
                       JM         flux_jacob[],
                       RV&        res)
  {
    const Real gamma_minus_3 = p.gamma - 3.;

    const Real uu = p.u * p.u;
    const Real uv = p.u * p.v;
    const Real uw = p.u * p.w;
    const Real vv = p.v * p.v;
    const Real vw = p.v * p.w;
    const Real ww = p.w * p.w;

    JM& Jx = flux_jacob[XX];

//  Jx(0,0) = 0.0;
    Jx(0,1) = 1.0;
//  Jx(0,2) = 0.0;
//  Jx(0,3) = 0.0;
//  Jx(0,4) = 0.0;
    Jx(1,0) = p.half_gm1_v2 - uu;
    Jx(1,1) = -gamma_minus_3*p.u;
    Jx(1,2) = -p.gamma_minus_1*p.v;
    Jx(1,3) = -p.gamma_minus_1*p.w;
    Jx(1,4) = p.gamma_minus_1;
    Jx(2,0) = -uv;
    Jx(2,1) = p.v;
    Jx(2,2) = p.u;
//  Jx(2,3) = 0.0;
//  Jx(2,4) = 0.0;
    Jx(3,0) = -uw;
    Jx(3,1) = p.w;
//  Jx(3,2) = 0.0;
    Jx(3,3) = p.u;
//  Jx(3,4) = 0.0;
    Jx(4,0) = p.u*(p.half_gm1_v2 - p.H);
    Jx(4,1) = -p.gamma_minus_1*uu + p.H;
    Jx(4,2) = -p.gamma_minus_1*uv;
    Jx(4,3) = -p.gamma_minus_1*uw;
    Jx(4,4) = p.gamma*p.u;

    JM& Jy = flux_jacob[YY];

//  Jy(0,0) = 0.0;
//  Jy(0,1) = 0.0;
    Jy(0,2) = 1.0;
//  Jy(0,3) = 0.0;
//  Jy(0,4) = 0.0;
    Jy(1,0) = -uv;
    Jy(1,1) = p.v;
    Jy(1,2) = p.u;
//  Jy(1,3) = 0.0;
//  Jy(1,4) = 0.0;
    Jy(2,0) = p.half_gm1_v2 - vv;
    Jy(2,1) = -p.gamma_minus_1*p.u;
    Jy(2,2) = -gamma_minus_3*p.v;
    Jy(2,3) = -p.gamma_minus_1*p.w;
    Jy(2,4) = p.gamma_minus_1;
    Jy(3,0) = -vw;
//  Jy(3,1) = 0.0;
    Jy(3,2) = p.w;
    Jy(3,3) = p.v;
//  Jy(3,4) = 0.0;
    Jy(4,0) = p.v*(p.half_gm1_v2 - p.H);
    Jy(4,1) = -p.gamma_minus_1*uv;
    Jy(4,2) = -p.gamma_minus_1*vv + p.H;
    Jy(4,3) = -p.gamma_minus_1*vw;
    Jy(4,4) = p.gamma*p.v;

    JM& Jz = flux_jacob[ZZ];

//  Jz(0,0) = 0.0;
//  Jz(0,1) = 0.0;
//  Jz(0,2) = 0.0;
    Jz(0,3) = 1.0;
//  Jz(0,4) = 0.0;
    Jz(1,0) = -uw;
    Jz(1,1) = p.w;
//  Jz(1,2) = 0.0;
    Jz(1,3) = p.u;
//  Jz(1,4) = 0.0;
    Jz(2,0) = -vw;
//  Jz(2,1) = 0.0;
    Jz(2,2) = p.w;
    Jz(2,3) = p.v;
//  Jz(2,4) = 0.0;
    Jz(3,0) = p.half_gm1_v2 - ww;
    Jz(3,1) = -p.gamma_minus_1*p.u;
    Jz(3,2) = -p.gamma_minus_1*p.v;
    Jz(3,3) = -gamma_minus_3*p.w;
    Jz(3,4) = p.gamma_minus_1;
    Jz(4,0) = p.w*(p.half_gm1_v2 - p.H);
    Jz(4,1) = -p.gamma_minus_1*uw;
    Jz(4,2) = -p.gamma_minus_1*vw;
    Jz(4,3) = -p.gamma_minus_1*ww + p.H;
    Jz(4,4) = p.gamma*p.w;

    res = Jx * p.grad_vars.col(XX) + Jy * p.grad_vars.col(YY) + Jz * p.grad_vars.col(ZZ);
  }

}; // Cons3D

////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // physics
} // cf3

#endif // cf3_physics_NavierStokes_Cons3D_hpp
