// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Physics_NavierStokes_Roe2D_hpp
#define CF_Physics_NavierStokes_Roe2D_hpp

#include <iostream>

#include "Common/StringConversion.hpp"
#include "Math/Defs.hpp"

#include "Physics/Variables.hpp"

#include "NavierStokes2D.hpp"

namespace CF {
namespace Physics {
namespace NavierStokes {

///////////////////////////////////////////////////////////////////////////////////////

class NavierStokes_API Roe2D : public VariablesT<Roe2D> {

public: //typedefs

  typedef NavierStokes2D     MODEL;

  enum { Z0 = 0, Z1 = 1, Z2 = 2, Z3 = 3 };

  typedef boost::shared_ptr<Roe2D> Ptr;
  typedef boost::shared_ptr<Roe2D const> ConstPtr;

public: // functions

  /// constructor
  /// @param name of the component
  Roe2D ( const std::string& name );

  /// virtual destructor
  virtual ~Roe2D();

  /// Get the class name
  static std::string type_name () { return "Roe2D"; }

  template <typename SV>
  static void compute_variables ( MODEL::Properties& p,
                                  SV& sol)
  {
    sol[Z0] = sqrt(p.rho);
    sol[Z1] = sol[Z0]*p.u;
    sol[Z2] = sol[Z0]*p.v;
    sol[Z3] = sol[Z0]*p.H;
  }

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

    p.rho = sol[Z0]*sol[Z0];
    p.u   = sol[Z1]/sol[Z0];
    p.v   = sol[Z2]/sol[Z0];
    p.H   = sol[Z3]/sol[Z0];

    p.uuvv = p.u*p.u + p.v*p.v;
    p.P = p.gamma_minus_1/p.gamma * p.rho*(p.H - 0.5*p.uuvv);

    p.rhou = p.rho * p.u;
    p.rhov = p.rho * p.v;
    p.rhoE = p.rho * p.E;


    p.inv_rho = 1. / p.rho;

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


      throw Common::BadValue( FromHere(), "Pressure is negative at coordinates ["
                                   + Common::to_str(coord[XX]) + ","
                                   + Common::to_str(coord[YY])
                                   + "]");
    }

    const Real RT = p.P * p.inv_rho;    // RT = p/rho

    p.E = p.rhoE * p.inv_rho;           // E = rhoE / rho

    p.a2 = p.gamma * RT;
    p.a = sqrt( p.a2 );

    p.Ma = sqrt( p.uuvv / p.a2 );

    p.T = RT / p.R;

    p.half_gm1_v2 = 0.5 * p.gamma_minus_1 * p.uuvv;
  }

  /// compute the physical flux
  template < typename FM >
  static void flux( const MODEL::Properties& p,
                    FM& flux)
  {
    throw Common::NotImplemented(FromHere(), "flux not implemented for Roe2D");
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv)
  {
    throw Common::NotImplemented(FromHere(), "flux_jacobian_eigen_values not implemented for Roe2D");
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV, typename OP >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv,
                                         OP& op )

  {
    throw Common::NotImplemented(FromHere(), "flux_jacobian_eigen_values not implemented for Roe2D");
  }

  /// decompose the eigen structure of the flux jacobians projected on the gradients
  template < typename GV, typename EM, typename EV >
  static void flux_jacobian_eigen_structure(const MODEL::Properties& p,
                                            const GV& direction,
                                            EM& Rv,
                                            EM& Lv,
                                            EV& Dv)
  {
    throw Common::NotImplemented(FromHere(), "flux_jacobian_eigen_structure not implemented for Roe2D");
  }

  /// compute the PDE residual
  template < typename JM, typename RV >
  static void residual(const MODEL::Properties& p,
                       JM         flux_jacob[],
                       RV&        res)
  {
    throw Common::NotImplemented(FromHere(), "flux_jacobian_eigen_structure not implemented for Roe2D");
  }

}; // Roe2D

////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // Physics
} // CF

#endif // CF_Physics_NavierStokes_Roe2D_hpp
