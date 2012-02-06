// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#ifndef cf3_physics_LinEuler_Cons2D_hpp
#define cf3_physics_LinEuler_Cons2D_hpp

#include "common/BasicExceptions.hpp"
#include "common/StringConversion.hpp"
#include "math/Defs.hpp"

#include "physics/Variables.hpp"

#include "LinEuler2D.hpp"

namespace cf3 {
namespace physics {
namespace LinEuler {

///////////////////////////////////////////////////////////////////////////////////////

/// Conservative variables for Linearized Euler 2D
/// @author Tiago Quintino
/// @author
class LinEuler_API Cons2D : public VariablesT<Cons2D> {

public: //typedefs

  typedef LinEuler2D     MODEL;

  enum { Rho = 0, Rho0U = 1, Rho0V = 2, P = 3 };

  
  

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

    p.rho   = sol[Rho];                 // acoustic density
    p.rho0u = sol[Rho0U];               // rho0.u
    p.rho0v = sol[Rho0V];               // rho0.v
    p.p     = sol[P];                   // acoustic pressure

    p.u = p.rho0u / p.rho0;                   // velocity along XX, rho0.u / rho0
    p.v = p.rho0v / p.rho0;                   // velocity along YY, rho0.v / rho0

    p.H = p.p/(p.rho+p.rho0) + 0.5*(p.u*p.u + p.v*p.v);

    if (p.P0 + p.p <= 0)
    {
      std::cout << "rho   : " << p.rho  << std::endl;
      std::cout << "rho0u : " << p.rho0u << std::endl;
      std::cout << "rho0v : " << p.rho0v << std::endl;
      std::cout << "P     : " << p.p    << std::endl;
      std::cout << "u     : " << p.u    << std::endl;
      std::cout << "v     : " << p.v    << std::endl;
      std::cout << "H     : " << p.H << std::endl;

      throw common::FailedToConverge( FromHere(), "Pressure is negative at coordinates ["
                               + common::to_str(coord[XX]) + ","
                               + common::to_str(coord[YY])
                               + "]");
    }

  }

  template < typename VectorT >
  static void compute_variables ( const MODEL::Properties& p, VectorT& vars )
  {
    vars[Rho]   = p.rho;
    vars[Rho0U] = p.rho0u;
    vars[Rho0V] = p.rho0v;
    vars[P]     = p.p;
  }

  /// compute the physical flux
  template < typename FM >
  static void flux( const MODEL::Properties& p,
                    FM& flux)
  {
//  From coolfluid 2
//    _fluxArray[varIDs[0]] = V0n*rho+un*rho0;
//    _fluxArray[varIDs[1]] = rho0*V0n*u+p*nx;
//    _fluxArray[varIDs[2]] = rho0*V0n*v+p*ny;
//    _fluxArray[varIDs[3]] = V0n*p+un*gamma*P0;

    flux(0,XX) = p.u0[XX]*p.rho+p.rho0u;
    flux(1,XX) = p.rho0*p.u0[XX]*p.u+p.p;
    flux(2,XX) = p.rho0*p.u0[XX]*p.v;
    flux(3,XX) = p.u0[XX]*p.p + p.u*p.gamma*p.P0;

    flux(0,YY) = p.u0[YY]*p.rho+p.rho0v;
    flux(1,YY) = p.rho0*p.u0[YY]*p.u;
    flux(2,YY) = p.rho0*p.u0[YY]*p.v+p.p;
    flux(3,YY) = p.u0[YY]*p.p + p.v*p.gamma*p.P0;
  }

  /// compute the physical flux
  template < typename FM , typename GV>
  static void flux( const MODEL::Properties& p,
                    const GV& direction,
                    FM& flux)
  {
    const Real u0n = p.u0[XX] * direction[XX] +
                     p.u0[YY] * direction[YY];

    const Real un = p.u * direction[XX] +
                    p.v * direction[YY];

    flux[0] = u0n*p.rho+p.rho0*un;
    flux[1] = p.rho0*u0n*p.u+p.p*direction[XX];
    flux[2] = p.rho0*u0n*p.v+p.p*direction[YY];
    flux[3] = u0n*p.p+un*p.gamma*p.P0;
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
//    throw common::NotImplemented(FromHere(), "Cons2D::residual()");

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
} // physics
} // cf3

#endif // cf3_physics_LinEuler_Cons2D_hpp
