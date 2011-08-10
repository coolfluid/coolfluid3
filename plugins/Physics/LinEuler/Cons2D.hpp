// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#ifndef CF_Physics_LinEuler_Cons2D_hpp
#define CF_Physics_LinEuler_Cons2D_hpp

#include "Common/BasicExceptions.hpp"
#include "Common/StringConversion.hpp"
#include "Math/Defs.hpp"

#include "Physics/Variables.hpp"

#include "LinEuler2D.hpp"

namespace CF {
namespace Physics {
namespace LinEuler {

///////////////////////////////////////////////////////////////////////////////////////

/// Conservative variables for Linearized Euler 2D
/// @author Tiago Quintino
/// @author
class LinEuler_API Cons2D : public VariablesT<Cons2D> {

public: //typedefs

  typedef LinEuler2D     MODEL;

  enum { Rho = 0, RhoU0 = 1, RhoV0 = 2, P = 3 };

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

    p.gamma = 1.0;

    p.rho0   = 1.0;                      // reference density
    p.u0[XX] = 0.5;                     // reference velocity
    p.u0[YY] = 0.0;
    p.P0     = 1.0;                      // reference pressure

    p.inv_rho0 = 1. / p.rho0;             // inverse of reference density, very commonly used

    p.c     = std::sqrt( p.gamma * p.P0 * p.inv_rho0 );

    p.inv_c = 1. / p.c;                 // inverse of the speed of sound, very commonly used

    p.rho   = sol[Rho];                 // acoustic density
    p.rho0u = sol[RhoU0];               // rho0.u
    p.rho0v = sol[RhoV0];               // rho0.v
    p.p     = sol[P];                   // acoustic pressure

    p.u = p.rho0u / p.rho0;                   // velocity along XX, rho0.u / rho0
    p.v = p.rho0v / p.rho0;                   // velocity along YY, rho0.v / rho0
  }

  /// compute the physical flux
  template < typename FM >
  static void flux( const MODEL::Properties& p,
                    FM& flux)
  {
//    flux(0,XX) = p.rhou;              // rho.u
//    flux(1,XX) = p.rhou * p.u + p.P;  // rho.u^2 + P
//    flux(2,XX) = p.rhou * p.v;        // rho.u.v
//    flux(3,XX) = p.rhou * p.H;        // rho.u.H

//    flux(0,YY) = p.rhov;              // rho.v
//    flux(1,YY) = p.rhov * p.u;        // rho.v.u
//    flux(2,YY) = p.rhov * p.v + p.P;  // rho.v^2 + P
//    flux(3,YY) = p.rhov * p.H;        // rho.v.H
    throw Common::NotImplemented(FromHere(), "Cons2D::flux()");
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv)
  {
    const Real Vn = p.u0[XX] * direction[XX] +
                    p.u0[YY] * direction[YY];

    Dv[0] = Vn;
    Dv[1] = Vn;
    Dv[2] = Vn + p.c;
    Dv[3] = Vn - p.c;
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV, typename OP >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv,
                                         OP& op )

  {
    const Real Vn = p.u0[XX] * direction[XX] +
                    p.u0[YY] * direction[YY];

    const Real op_um = op(Vn);

    Dv[0] = op_um;
    Dv[1] = op_um;
    Dv[2] = op_um + p.c;
    Dv[3] = op_um - p.c;
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

    // state is not used as Linearized Euler is, well, linear

    const Real Vn = p.u0[XX] * direction[XX] +
                    p.u0[YY] * direction[YY];

    const Real inv_c2  = p.inv_c / p.c;

    Rv(0,0) = 1.0;
    Rv(0,1) = 0.0;
    Rv(0,2) = 0.5*p.inv_c;
    Rv(0,3) = 0.5*p.inv_c;

    Rv(1,0) = 0.0;
    Rv(1,1) = ny;
    Rv(1,2) = 0.5*nx;
    Rv(1,3) = -0.5*nx;

    Rv(2,0) = 0.0;
    Rv(2,1) = -nx;
    Rv(2,2) = 0.5*ny;
    Rv(2,3) = -0.5*ny;

    Rv(3,0) = 0.0;
    Rv(3,1) = 0.0;
    Rv(3,2) = 0.5*p.c;
    Rv(3,3) = 0.5*p.c;

    Lv(0,0) = 1.0;
    Lv(0,1) = 0.0;
    Lv(0,2) = 0.0;
    Lv(0,3) = -inv_c2;

    Lv(1,0) = 0.0;
    Lv(1,1) = ny;
    Lv(1,2) = -nx;
    Lv(1,3) = 0.0;

    Lv(2,0) = 0.0;
    Lv(2,1) = nx;
    Lv(2,2) = ny;
    Lv(2,3) = p.inv_c;

    Lv(3,0) = 0.0;
    Lv(3,1) = -nx;
    Lv(3,2) = -ny;
    Lv(3,3) = p.inv_c;

    Dv[0] = Vn;
    Dv[1] = Vn;
    Dv[2] = Vn + p.c;
    Dv[3] = Vn - p.c;
  }

  /// compute the PDE residual
  template < typename JM, typename RV >
  static void residual(const MODEL::Properties& p,
                       JM         flux_jacob[],
                       RV&        res)
  {
//    throw Common::NotImplemented(FromHere(), "Cons2D::residual()");

//    const Real gamma_minus_3 = p.gamma - 3.;

//    const Real uu = p.u * p.u;
//    const Real uv = p.u * p.v;
//    const Real vv = p.v * p.v;

    JM& A = flux_jacob[XX];

  //    A.setZero(); // assume are zeroed

    A(0,0) = p.u0[XX];
    A(0,1) = 1.;
    A(0,2) = 0.;
    A(0,3) = 0.;
    A(1,0) = 0.;
    A(1,1) = p.u0[XX];
    A(1,2) = 0.;
    A(1,3) = 1.;
    A(2,0) = 0.;
    A(2,1) = 0.;
    A(2,2) = p.u0[XX];
    A(2,3) = 0.;
    A(3,0) = 0.;
    A(3,1) = p.c * p.c;
    A(3,2) = 0.;
    A(3,3) = p.u0[XX];

    JM& B = flux_jacob[YY];

    //    B.setZero(); // assume are zeroed

    B(0,0) = p.u0[YY];
    B(0,1) = 0.;
    B(0,2) = 1.;
    B(0,3) = 0.;
    B(1,0) = 0.;
    B(1,1) = p.u0[YY];
    B(1,2) = 0.;
    B(1,3) = 0.;
    B(2,0) = 0.;
    B(2,1) = 0.;
    B(2,2) = p.u0[YY];
    B(2,3) = 1.;
    B(3,0) = 0.;
    B(3,1) = 0.;
    B(3,2) = p.c * p.c;
    B(3,3) = p.u0[YY];

    res = A * p.grad_vars.col(XX) + B * p.grad_vars.col(YY);

  }

}; // Cons2D

////////////////////////////////////////////////////////////////////////////////////

} // LinEuler
} // Physics
} // CF

#endif // CF_Physics_LinEuler_Cons2D_hpp
