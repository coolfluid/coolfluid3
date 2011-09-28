// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Physics_NavierStokes_Prim1D_hpp
#define CF_Physics_NavierStokes_Prim1D_hpp

#include <iostream>

#include "Common/StringConversion.hpp"
#include "Math/Defs.hpp"

#include "Physics/Variables.hpp"

#include "NavierStokes1D.hpp"

namespace CF {
namespace Physics {
namespace NavierStokes {

///////////////////////////////////////////////////////////////////////////////////////

class NavierStokes_API Prim1D : public VariablesT<Prim1D> {

public: //typedefs

  typedef NavierStokes1D     MODEL;

  enum { Rho = 0, U = 1, P = 2 };

  typedef boost::shared_ptr<Prim1D> Ptr;
  typedef boost::shared_ptr<Prim1D const> ConstPtr;

public: // functions

  /// constructor
  /// @param name of the component
  Prim1D ( const std::string& name );

  /// virtual destructor
  virtual ~Prim1D();

  /// Get the class name
  static std::string type_name () { return "Prim1D"; }

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

    p.R = 287.058;                 // air
    p.gamma = 1.4;                 // diatomic ideal gas
    p.gamma_minus_1 = p.gamma - 1.;

    p.rho = sol[Rho];
    p.u   = sol[U];
    p.P   = sol[P];

    p.inv_rho = 1. / p.rho;
    p.uu = p.u*p.u;
    p.H = p.P * p.inv_rho*p.gamma/p.gamma_minus_1 + 0.5*p.uu;


    const Real RT = p.P * p.inv_rho;    // RT = p/rho
    p.E = p.H - RT;
    p.rhou = p.rho * p.u;
    p.rhoE = p.rho * p.E;


    if( p.P <= 0. )
    {
          std::cout << "rho   : " << p.rho  << std::endl;
          std::cout << "rhou  : " << p.rhou << std::endl;
          std::cout << "rhoE  : " << p.rhoE << std::endl;
          std::cout << "P     : " << p.P    << std::endl;
          std::cout << "u     : " << p.u    << std::endl;
          std::cout << "uu    : " << p.uu   << std::endl;


      throw Common::BadValue( FromHere(), "Pressure is negative at coordinates ["
                                   + Common::to_str(coord[XX])
                                   + "]");
    }

    p.a2 = p.gamma * RT;
    p.a = sqrt( p.a2 );

    p.Ma = sqrt( p.uu / p.a2 );

    p.T = RT / p.R;

    p.half_gm1_v2 = 0.5 * p.gamma_minus_1 * p.uu;
  }

  template < typename VectorT >
  static void compute_variables ( const MODEL::Properties& p, VectorT& vars )
  {
    vars[Rho] = p.rho;
    vars[U]   = p.u;
    vars[P]   = p.P;
  }


  /// compute the physical flux
  template < typename FM >
  static void flux( const MODEL::Properties& p,
                    FM& flux)
  {
    throw Common::NotImplemented(FromHere(), "flux not implemented for Prim1D");
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv)
  {
    throw Common::NotImplemented(FromHere(), "flux_jacobian_eigen_values not implemented for Prim1D");
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV, typename OP >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv,
                                         OP& op )

  {
    throw Common::NotImplemented(FromHere(), "flux_jacobian_eigen_values not implemented for Prim1D");
  }

  /// decompose the eigen structure of the flux jacobians projected on the gradients
  template < typename GV, typename EM, typename EV >
  static void flux_jacobian_eigen_structure(const MODEL::Properties& p,
                                            const GV& direction,
                                            EM& Rv,
                                            EM& Lv,
                                            EV& Dv)
  {
    throw Common::NotImplemented(FromHere(), "flux_jacobian_eigen_structure not implemented for Prim1D");
  }

  /// compute the PDE residual
  template < typename JM, typename RV >
  static void residual(const MODEL::Properties& p,
                       JM         flux_jacob[],
                       RV&        res)
  {
    throw Common::NotImplemented(FromHere(), "flux_jacobian_eigen_structure not implemented for Prim1D");
  }

}; // Prim1D

////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // Physics
} // CF

#endif // CF_Physics_NavierStokes_Prim1D_hpp
