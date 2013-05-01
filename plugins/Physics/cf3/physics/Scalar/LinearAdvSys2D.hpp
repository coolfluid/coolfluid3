// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_physics_Scalar_LinearAdvSys2D_hpp
#define cf3_physics_Scalar_LinearAdvSys2D_hpp

#include "cf3/common/StringConversion.hpp"
#include "math/Defs.hpp"

#include "cf3/physics/Variables.hpp"

#include "cf3/physics/Scalar/ScalarSys2D.hpp"

namespace cf3 {
namespace physics {
namespace Scalar {

///////////////////////////////////////////////////////////////////////////////////////

class Scalar_API LinearAdvSys2D : public VariablesT<LinearAdvSys2D> {

public: //typedefs

  typedef ScalarSys2D     MODEL;

  enum { U0 = 0, U1 = 1 };

  
  

public: // functions

  /// constructor
  /// @param name of the component
  LinearAdvSys2D ( const std::string& name );

  /// virtual destructor
  virtual ~LinearAdvSys2D();

  /// Get the class name
  static std::string type_name () { return "LinearAdvSys2D"; }

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

    p.v(U0,XX) = 1.0;
    p.v(U0,YY) = 1.0;

    p.v(U1,XX) = 1.0;
    p.v(U1,YY) = 0.5;

    p.u0 = sol[U0];
    p.u1 = sol[U1];

    // no diffusion

    p.mu[XX] = 0.;
    p.mu[YY] = 0.;
  }

  template < typename VectorT >
  static void compute_variables ( const MODEL::Properties& p, VectorT& vars )
  {
    vars[U0]  = p.u0;
    vars[U1]  = p.u1;
  }

  /// compute the physical flux
  template < typename FM >
  static void flux( const MODEL::Properties& p,
                    FM& flux)
  {
    flux.row(U0)  = p.v.row(U0) * p.vars[U0];
    flux.row(U1)  = p.v.row(U1) * p.vars[U0];
  }

  /// compute the physical flux
  template < typename FM , typename GV>
  static void flux( const MODEL::Properties& p,
                    const GV& direction,
                    FM& flux)
  {
    throw common::NotImplemented(FromHere(), "directional flux not implemented for LinearAdvSys2D");
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv)
  {
    Dv = p.v * direction;
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV, typename OP >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv,
                                         OP& op )

  {
    Dv = p.v * direction;
    Dv = Dv.unaryExpr( op );
  }

  /// decompose the eigen structure of the flux jacobians projected on the gradients
  template < typename GV, typename EM, typename EV >
  static void flux_jacobian_eigen_structure(const MODEL::Properties& p,
                                            const GV& direction,
                                            EM& Rv,
                                            EM& Lv,
                                            EV& Dv)
  {
    Rv.setIdentity();
    Lv.setIdentity();

    Dv = p.v * direction;
  }

  /// compute the PDE residual
  template < typename JM, typename RV >
  static void residual(const MODEL::Properties& p,
                       JM         flux_jacob[],
                       RV&        res)
  {
    JM& Jx = flux_jacob[XX];
    JM& Jy = flux_jacob[YY];

    Jx.diagonal() = p.v.col(XX);
    Jy.diagonal() = p.v.col(YY);

    res = Jx * p.grad_vars.col(XX) + Jy * p.grad_vars.col(YY);
  }

}; // LinearAdvSys2D

////////////////////////////////////////////////////////////////////////////////////

} // Scalar
} // physics
} // cf3

#endif // cf3_physics_Scalar_LinearAdvSys2D_hpp
