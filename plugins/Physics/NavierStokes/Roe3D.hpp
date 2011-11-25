// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_physics_NavierStokes_Roe3D_hpp
#define cf3_physics_NavierStokes_Roe3D_hpp

#include <iostream>

#include "common/StringConversion.hpp"
#include "math/Defs.hpp"

#include "physics/Variables.hpp"

#include "NavierStokes3D.hpp"

namespace cf3 {
namespace physics {
namespace NavierStokes {

///////////////////////////////////////////////////////////////////////////////////////

class NavierStokes_API Roe3D : public VariablesT<Roe3D> {

public: //typedefs

  typedef NavierStokes3D     MODEL;

  enum { Z0 = 0, Z1 = 1, Z2 = 2, Z3 = 3 , Z4 = 4 };

  typedef boost::shared_ptr<Roe3D> Ptr;
  typedef boost::shared_ptr<Roe3D const> ConstPtr;

public: // functions

  /// constructor
  /// @param name of the component
  Roe3D ( const std::string& name );

  /// virtual destructor
  virtual ~Roe3D();

  /// Get the class name
  static std::string type_name () { return "Roe3D"; }

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

    p.rho = sol[Z0]*sol[Z0];
    p.u   = sol[Z1]/sol[Z0];
    p.v   = sol[Z2]/sol[Z0];
    p.w   = sol[Z3]/sol[Z0];
    p.H   = sol[Z4]/sol[Z0];

    p.uuvvww = p.u*p.u + p.v*p.v + p.w*p.w;
    p.P = p.gamma_minus_1/p.gamma * p.rho*(p.H - 0.5*p.uuvvww);

    p.rhou = p.rho * p.u;
    p.rhov = p.rho * p.v;
    p.rhow = p.rho * p.w;
    p.rhoE = p.rho * p.E;


    p.inv_rho = 1. / p.rho;

    if( p.P <= 0. )
    {
          std::cout << "rho    : " << p.rho  << std::endl;
          std::cout << "rhou   : " << p.rhou << std::endl;
          std::cout << "rhov   : " << p.rhov << std::endl;
          std::cout << "rhoE   : " << p.rhoE << std::endl;
          std::cout << "P      : " << p.P    << std::endl;
          std::cout << "u      : " << p.u    << std::endl;
          std::cout << "v      : " << p.v    << std::endl;
          std::cout << "w      : " << p.w    << std::endl;
          std::cout << "uuvvww : " << p.uuvvww << std::endl;


      throw common::BadValue( FromHere(), "Pressure is negative at coordinates ["
                                   + common::to_str(coord[XX]) + ","
                                   + common::to_str(coord[YY]) + ","
                                   + common::to_str(coord[ZZ])
                                   + "]");
    }

    const Real RT = p.P * p.inv_rho;    // RT = p/rho

    p.E = p.rhoE * p.inv_rho;           // E = rhoE / rho

    p.a2 = p.gamma * RT;
    p.a = sqrt( p.a2 );

    p.Ma = sqrt( p.uuvvww / p.a2 );

    p.T = RT / p.R;

    p.half_gm1_v2 = 0.5 * p.gamma_minus_1 * p.uuvvww;
  }

  template < typename VectorT >
  static void compute_variables ( const MODEL::Properties& p, VectorT& vars )
  {
    vars[Z0] = sqrt(p.rho);
    vars[Z1] = vars[Z0]*(p.u);
    vars[Z2] = vars[Z0]*(p.v);
    vars[Z3] = vars[Z0]*(p.w);
    vars[Z4] = vars[Z0]*(p.H);
  }

  /// compute the physical flux
  template < typename FM >
  static void flux( const MODEL::Properties& p,
                    FM& flux)
  {
    throw common::NotImplemented(FromHere(), "flux not implemented for Roe3D");
  }


  /// compute the physical flux
  template < typename FM , typename GV>
  static void flux( const MODEL::Properties& p,
                    const GV& direction,
                    FM& flux)
  {
    throw common::NotImplemented(FromHere(), "flux not implemented for Roe3D");
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv)
  {
    throw common::NotImplemented(FromHere(), "flux_jacobian_eigen_values not implemented for Roe3D");
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV, typename OP >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv,
                                         OP& op )

  {
    throw common::NotImplemented(FromHere(), "flux_jacobian_eigen_values not implemented for Roe3D");
  }

  /// decompose the eigen structure of the flux jacobians projected on the gradients
  template < typename GV, typename EM, typename EV >
  static void flux_jacobian_eigen_structure(const MODEL::Properties& p,
                                            const GV& direction,
                                            EM& Rv,
                                            EM& Lv,
                                            EV& Dv)
  {
    throw common::NotImplemented(FromHere(), "flux_jacobian_eigen_structure not implemented for Roe3D");
  }

  /// compute the PDE residual
  template < typename JM, typename RV >
  static void residual(const MODEL::Properties& p,
                       JM         flux_jacob[],
                       RV&        res)
  {
    throw common::NotImplemented(FromHere(), "flux_jacobian_eigen_structure not implemented for Roe3D");
  }

}; // Roe3D

////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // physics
} // cf3

#endif // cf3_physics_NavierStokes_Roe3D_hpp
