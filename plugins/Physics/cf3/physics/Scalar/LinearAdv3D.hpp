// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_physics_Scalar_LinearAdv3D_hpp
#define cf3_physics_Scalar_LinearAdv3D_hpp

#include "cf3/common/StringConversion.hpp"
#include "math/Defs.hpp"

#include "cf3/physics/Variables.hpp"

#include "cf3/physics/Scalar/Scalar3D.hpp"

namespace cf3 {
namespace physics {
namespace Scalar {

///////////////////////////////////////////////////////////////////////////////////////

class Scalar_API LinearAdv3D : public VariablesT<LinearAdv3D> {

public: //typedefs

  typedef Scalar3D     MODEL;

  enum { U = 0 };

  
  

public: // functions

  /// constructor
  /// @param name of the component
  LinearAdv3D ( const std::string& name );

  /// virtual destructor
  virtual ~LinearAdv3D();

  /// Get the class name
  static std::string type_name () { return "LinearAdv3D"; }

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

    p.v[XX] = 1.0; // constant vx
    p.v[YY] = 1.0; // constant vy
    p.v[ZZ] = 0.0; // constant vz

    p.u = sol[U];

    p.mu = 0.;     // no diffusion
  }

  template < typename VectorT >
  static void compute_variables ( const MODEL::Properties& p, VectorT& vars )
  {
    vars[U]  = p.u;
  }

  /// compute the physical flux
  template < typename FM >
  static void flux( const MODEL::Properties& p,
                    FM& flux)
  {
    flux(0,XX)   = p.v[XX] * p.u;
    flux(0,YY)   = p.v[YY] * p.u;
    flux(0,ZZ)   = p.v[ZZ] * p.u;
  }

  /// compute the physical flux
  template < typename FM , typename GV>
  static void flux( const MODEL::Properties& p,
                    const GV& direction,
                    FM& flux)
  {
    flux[0] = p.u * (p.v[XX] * direction[XX] +
                     p.v[YY] * direction[YY] +
                     p.v[ZZ] * direction[ZZ]);
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv)
  {
    Dv[0]   = p.v[XX] * direction[XX]
            + p.v[YY] * direction[YY]
            + p.v[ZZ] * direction[ZZ];
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV, typename OP >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv,
                                         OP& op )

  {
    Dv[0] = op( p.v[XX] * direction[XX]
              + p.v[YY] * direction[YY]
              + p.v[ZZ] * direction[ZZ] );
  }

  /// decompose the eigen structure of the flux jacobians projected on the gradients
  template < typename GV, typename EM, typename EV >
  static void flux_jacobian_eigen_structure(const MODEL::Properties& p,
                                            const GV& direction,
                                            EM& Rv,
                                            EM& Lv,
                                            EV& Dv)
  {
    Rv(0,0) = 1.;
    Lv(0,0) = 1.;
    Dv[0]   = p.v[XX] * direction[XX]
            + p.v[YY] * direction[YY]
            + p.v[ZZ] * direction[ZZ];
  }

  /// compute the PDE residual
  template < typename JM, typename RV >
  static void residual(const MODEL::Properties& p,
                       JM         flux_jacob[],
                       RV&        res)
  {
    JM& Jx = flux_jacob[XX];
    JM& Jy = flux_jacob[YY];
    JM& Jz = flux_jacob[ZZ];

    Jx(0,0) = p.v[XX];
    Jy(0,0) = p.v[YY];
    Jz(0,0) = p.v[ZZ];

    res = Jx * p.grad_vars.col(XX)
        + Jy * p.grad_vars.col(YY)
        + Jz * p.grad_vars.col(ZZ);
  }

}; // LinearAdv3D

////////////////////////////////////////////////////////////////////////////////////

} // Scalar
} // physics
} // cf3

#endif // cf3_physics_Scalar_LinearAdv3D_hpp
