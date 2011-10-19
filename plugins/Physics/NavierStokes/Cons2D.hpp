// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Physics_NavierStokes_Cons2D_hpp
#define cf3_Physics_NavierStokes_Cons2D_hpp

#include <iostream>

#include "common/StringConversion.hpp"
#include "Math/Defs.hpp"

#include "Physics/Variables.hpp"

#include "NavierStokes2D.hpp"

namespace cf3 {
namespace Physics {
namespace NavierStokes {

///////////////////////////////////////////////////////////////////////////////////////

class NavierStokes_API Cons2D : public VariablesT<Cons2D> {

public: //typedefs

  typedef NavierStokes2D     MODEL;

  enum { Rho = 0, RhoU = 1, RhoV = 2, RhoE = 3 };

  typedef boost::shared_ptr<Cons2D> Ptr;
  typedef boost::shared_ptr<Cons2D const> ConstPtr;

public: // functions

  /// constructor
  /// @param name of the component
  Cons2D ( const std::string& name );

  /// virtual destructor
  virtual ~Cons2D();

  /// Get the class name
  static std::string type_name () { return "Cons2D"; }

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
    p.rhoE  = sol[RhoE];

    p.inv_rho = 1. / p.rho;

    p.u   = p.rhou * p.inv_rho;
    p.v   = p.rhov * p.inv_rho;

    p.uuvv = p.u*p.u + p.v*p.v;

    p.P = p.gamma_minus_1 * ( p.rhoE - 0.5 * p.rho * p.uuvv );

    if( p.P <= 0. )
    {
          std::cout << "rho   : " << p.rho  << std::endl;
          std::cout << "rhou  : " << p.rhou << std::endl;
          std::cout << "rhov  : " << p.rhov << std::endl;
          std::cout << "rhoE  : " << p.rhoE << std::endl;
          std::cout << "P     : " << p.P    << std::endl;
          std::cout << "u     : " << p.u    << std::endl;
          std::cout << "v     : " << p.v    << std::endl;
          std::cout << "uuvv  : " << p.uuvv << std::endl;


      throw common::BadValue( FromHere(), "Pressure is negative at coordinates ["
                                   + common::to_str(coord[XX]) + ","
                                   + common::to_str(coord[YY])
                                   + "]");
    }

    const Real RT = p.P * p.inv_rho;    // RT = p/rho

    p.E = p.rhoE * p.inv_rho;           // E = rhoE / rho

    p.H = p.E + RT;                     // H = E + p/rho

    p.a = sqrt( p.gamma * RT );

    p.a2 = p.a * p.a;

    p.Ma = sqrt( p.uuvv / p.a2 );

    p.T = RT / p.R;

    p.half_gm1_v2 = 0.5 * p.gamma_minus_1 * p.uuvv;
  }

  template < typename VectorT >
  static void compute_variables ( const MODEL::Properties& p, VectorT& vars )
  {
    vars[Rho]  = p.rho;
    vars[RhoU] = p.rhou;
    vars[RhoV] = p.rhov;
    vars[RhoE] = p.rhoE;
  }

  /// compute the physical flux
  template < typename FM >
  static void flux( const MODEL::Properties& p,
                    FM& flux)
  {
    flux(0,XX) = p.rhou;              // rho.u
    flux(1,XX) = p.rhou * p.u + p.P;  // rho.u^2 + P
    flux(2,XX) = p.rhou * p.rhov * p.inv_rho; // rho.u.v
    flux(3,XX) = p.rhou * p.H;        // rho.u.H

    flux(0,YY) = p.rhov;              // rho.v
    flux(1,YY) = p.rhou * p.rhov * p.inv_rho; // rho.v.u
    flux(2,YY) = p.rhov * p.v + p.P;  // rho.v^2 + P
    flux(3,YY) = p.rhov * p.H;        // rho.v.H
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv)
  {
    const Real um = p.u * direction[XX]
                  + p.v * direction[YY];

    Dv[0] = um;
    Dv[1] = um;
    Dv[2] = um + p.a;
    Dv[3] = um - p.a;
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV, typename OP >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv,
                                         OP& op )

  {
    const Real um = p.u * direction[XX]
                  + p.v * direction[YY];

    const Real op_um = op(um);

    Dv[0] = op_um;
    Dv[1] = op_um;
    Dv[2] = op_um + p.a;
    Dv[3] = op_um - p.a;
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

    const Real inv_a  = 1. / p.a;
    const Real inv_a2 = inv_a * inv_a;

    const Real um = p.u * nx + p.v * ny;
    const Real ra = 0.5 * p.rho * inv_a;

    const Real coeffM2 = p.half_gm1_v2 * inv_a2;
    const Real uDivA = p.gamma_minus_1 * p.u * inv_a;
    const Real vDivA = p.gamma_minus_1 * p.v * inv_a;
    const Real rho_a = p.rho * p.a;

    const Real gm1_ov_rhoa = p.gamma_minus_1 / rho_a;

    // matrix of right eigen vectors R

    Rv(0,0) = 1.;
    Rv(0,1) = 0.;
    Rv(0,2) = ra;
    Rv(0,3) = ra;
    Rv(1,0) = p.u;
    Rv(1,1) = p.rho * ny;
    Rv(1,2) = ra*(p.u + p.a*nx);
    Rv(1,3) = ra*(p.u - p.a*nx);
    Rv(2,0) = p.v;
    Rv(2,1) = -p.rho*nx;
    Rv(2,2) = ra*(p.v + p.a*ny);
    Rv(2,3) = ra*(p.v - p.a*ny);
    Rv(3,0) = 0.5 * p.uuvv;
    Rv(3,1) = p.rho * (p.u*ny - p.v*nx);
    Rv(3,2) = ra*(p.H + p.a*um);
    Rv(3,3) = ra*(p.H - p.a*um);

    // matrix of left eigen vectors L = R.inverse();

    Lv(0,0) = 1.- coeffM2;
    Lv(0,1) = uDivA*inv_a;
    Lv(0,2) = vDivA*inv_a;
    Lv(0,3) = -p.gamma_minus_1 * inv_a2;
    Lv(1,0) = p.inv_rho * (p.v*nx - p.u*ny);
    Lv(1,1) = p.inv_rho * ny;
    Lv(1,2) = -p.inv_rho * nx;
    Lv(1,3) = 0.0;
    Lv(2,0) = p.a*p.inv_rho * (coeffM2 - um*inv_a);
    Lv(2,1) = p.inv_rho * (nx - uDivA);
    Lv(2,2) = p.inv_rho * (ny - vDivA);
    Lv(2,3) = gm1_ov_rhoa;
    Lv(3,0) = p.a*p.inv_rho*(coeffM2 + um*inv_a);
    Lv(3,1) = -p.inv_rho*(nx + uDivA);
    Lv(3,2) = -p.inv_rho*(ny + vDivA);
    Lv(3,3) = gm1_ov_rhoa;

    // diagonal matrix of eigen values

    Dv[0] = um;
    Dv[1] = um;
    Dv[2] = um + p.a;
    Dv[3] = um - p.a;

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
    const Real vv = p.v * p.v;

    JM& A = flux_jacob[XX];

    //    A.setZero(); // assume are zeroed

    A(0,0) = 0.;
    A(0,1) = 1.;
    A(0,2) = 0.;
    A(0,3) = 0.;

    A(1,0) = p.half_gm1_v2 - uu;
    A(1,1) = -gamma_minus_3*p.u;
    A(1,2) = -p.gamma_minus_1*p.v;
    A(1,3) = p.gamma_minus_1;

    A(2,0) = -uv;
    A(2,1) = p.v;
    A(2,2) = p.u;
    A(2,3) = 0.;

    A(3,0) = p.half_gm1_v2*p.u - p.u * p.H;
    A(3,1) = -p.gamma_minus_1*uu + p.H;
    A(3,2) = -p.gamma_minus_1*uv;
    A(3,3) = p.gamma*p.u;

    JM& B = flux_jacob[YY];

    //    B.setZero(); // assume are zeroed

    B(0,0) = 0.;
    B(0,1) = 0.;
    B(0,2) = 1.;
    B(0,3) = 0.;

    B(1,0) = -uv;
    B(1,1) = p.v;
    B(1,2) = p.u;
    B(1,3) = 0.;

    B(2,0) = p.half_gm1_v2 - vv;
    B(2,1) = -p.gamma_minus_1*p.u;
    B(2,2) = -gamma_minus_3*p.v;
    B(2,3) = p.gamma_minus_1;

    B(3,0) = p.half_gm1_v2*p.v - p.v*p.H;
    B(3,1) = -p.gamma_minus_1*uv;
    B(3,2) = -p.gamma_minus_1*vv + p.H;
    B(3,3) = p.gamma*p.v;

    res = A * p.grad_vars.col(XX) + B * p.grad_vars.col(YY);
  }

}; // Cons2D

////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // Physics
} // cf3

#endif // cf3_Physics_NavierStokes_Cons2D_hpp
