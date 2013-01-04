// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_physics_NavierStokes_Prim3D_hpp
#define cf3_physics_NavierStokes_Prim3D_hpp

#include <iostream>

#include "cf3/common/StringConversion.hpp"
#include "cf3/math/Defs.hpp"

#include "cf3/physics/Variables.hpp"

#include "cf3/physics/NavierStokes/NavierStokes3D.hpp"

namespace cf3 {
namespace physics {
namespace NavierStokes {

///////////////////////////////////////////////////////////////////////////////////////

class NavierStokes_API Prim3D : public VariablesT<Prim3D> {

public: //typedefs

  typedef NavierStokes3D     MODEL;

  enum { Rho = 0, U = 1, V = 2, W=3, P = 4 };

  
  

public: // functions

  /// constructor
  /// @param name of the component
  Prim3D ( const std::string& name );

  /// virtual destructor
  virtual ~Prim3D();

  /// Get the class name
  static std::string type_name () { return "Prim3D"; }

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

    p.rho = sol[Rho];
    p.u   = sol[U];
    p.v   = sol[V];
    p.w   = sol[W];
    p.P   = sol[P];

    p.inv_rho = 1. / p.rho;
    p.uuvvww = p.u*p.u + p.v*p.v + p.w*p.w;
    p.H = p.P * p.inv_rho*p.gamma/p.gamma_minus_1 + 0.5*p.uuvvww;


    const Real RT = p.P * p.inv_rho;    // RT = p/rho
    p.E = p.H - RT;
    p.rhou = p.rho * p.u;
    p.rhov = p.rho * p.v;
    p.rhow = p.rho * p.w;
    p.rhoE = p.rho * p.E;


    if( p.P <= 0. )
    {
          std::cout << "rho     : " << p.rho    << std::endl;
          std::cout << "rhou    : " << p.rhou   << std::endl;
          std::cout << "rhov    : " << p.rhov   << std::endl;
          std::cout << "rhow    : " << p.rhow   << std::endl;
          std::cout << "rhoE    : " << p.rhoE   << std::endl;
          std::cout << "P       : " << p.P      << std::endl;
          std::cout << "u       : " << p.u      << std::endl;
          std::cout << "v       : " << p.v      << std::endl;
          std::cout << "w       : " << p.w      << std::endl;
          std::cout << "uuvvww  : " << p.uuvvww << std::endl;


      throw common::FailedToConverge( FromHere(), "Pressure is negative at coordinates ["
                                   + common::to_str(coord[XX]) + ","
                                   + common::to_str(coord[YY])
                                   + common::to_str(coord[ZZ])
                                   + "]");
    }

    p.a2 = p.gamma * RT;
    p.a = sqrt( p.a2 );

    p.Ma = sqrt( p.uuvvww / p.a2 );

    p.T = RT / p.R;

    p.half_gm1_v2 = 0.5 * p.gamma_minus_1 * p.uuvvww;
  }

  template < typename VectorT >
  static void compute_variables ( const MODEL::Properties& p, VectorT& vars )
  {
    vars[Rho] = p.rho;
    vars[U]   = p.u;
    vars[V]   = p.v;
    vars[W]   = p.w;
    vars[P]   = p.P;
  }


  /// compute the physical flux
  template < typename FM >
  static void flux( const MODEL::Properties& p,
                    FM& flux)
  {
    throw common::NotImplemented(FromHere(), "flux not implemented for Prim3D");
  }

  /// compute the physical flux
  template < typename FM , typename GV>
  static void flux( const MODEL::Properties& p,
                    const GV& direction,
                    FM& flux)
  {
    throw common::NotImplemented(FromHere(), "flux not implemented for Prim3D");
  }


  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv)
  {
    throw common::NotImplemented(FromHere(), "flux_jacobian_eigen_values not implemented for Prim3D");
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV, typename OP >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv,
                                         OP& op )

  {
    throw common::NotImplemented(FromHere(), "flux_jacobian_eigen_values not implemented for Prim3D");
  }

  /// decompose the eigen structure of the flux jacobians projected on the gradients
  template < typename GV, typename EM, typename EV >
  static void flux_jacobian_eigen_structure(const MODEL::Properties& p,
                                            const GV& direction,
                                            EM& Rv,
                                            EM& Lv,
                                            EV& Dv)
  {
    throw common::NotImplemented(FromHere(), "flux_jacobian_eigen_structure not implemented for Prim3D");
  }

  /// compute the PDE residual
  template < typename JM, typename RV >
  static void residual(const MODEL::Properties& p,
                       JM         flux_jacob[],
                       RV&        res)
  {
    throw common::NotImplemented(FromHere(), "flux_jacobian_eigen_structure not implemented for Prim3D");
  }

}; // Prim3D

////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // physics
} // cf3

#endif // cf3_physics_NavierStokes_Prim3D_hpp
