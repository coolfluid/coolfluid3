// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Physics_NavierStokes_Roe1D_hpp
#define cf3_Physics_NavierStokes_Roe1D_hpp

#include <iostream>

#include "common/StringConversion.hpp"
#include "math/Defs.hpp"

#include "Physics/Variables.hpp"

#include "NavierStokes1D.hpp"

namespace cf3 {
namespace Physics {
namespace NavierStokes {

///////////////////////////////////////////////////////////////////////////////////////

class NavierStokes_API Roe1D : public VariablesT<Roe1D> {

public: //typedefs

  typedef NavierStokes1D     MODEL;

  enum { Z0 = 0, Z1 = 1, Z2 = 2 };

  typedef boost::shared_ptr<Roe1D> Ptr;
  typedef boost::shared_ptr<Roe1D const> ConstPtr;

public: // functions

  /// constructor
  /// @param name of the component
  Roe1D ( const std::string& name );

  /// virtual destructor
  virtual ~Roe1D();

  /// Get the class name
  static std::string type_name () { return "Roe1D"; }

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
    p.H   = sol[Z2]/sol[Z0];

    p.uu = p.u*p.u;
    p.P = p.gamma_minus_1/p.gamma * p.rho*(p.H - 0.5*p.uu);

    p.rhou = p.rho * p.u;
    p.rhoE = p.rho * p.E;


    p.inv_rho = 1. / p.rho;

    if( p.P <= 0. )
    {
          std::cout << "rho   : " << p.rho  << std::endl;
          std::cout << "rhou  : " << p.rhou << std::endl;
          std::cout << "rhoE  : " << p.rhoE << std::endl;
          std::cout << "P     : " << p.P    << std::endl;
          std::cout << "u     : " << p.u    << std::endl;
          std::cout << "uu    : " << p.uu   << std::endl;


      throw common::BadValue( FromHere(), "Pressure is negative at coordinates ["
                                   + common::to_str(coord[XX])
                                   + "]");
    }

    const Real RT = p.P * p.inv_rho;    // RT = p/rho

    p.E = p.rhoE * p.inv_rho;           // E = rhoE / rho

    p.a2 = p.gamma * RT;
    p.a = sqrt( p.a2 );

    p.Ma = sqrt( p.uu / p.a2 );

    p.T = RT / p.R;

    p.half_gm1_v2 = 0.5 * p.gamma_minus_1 * p.uu;
  }

  template < typename VectorT >
  static void compute_variables ( const MODEL::Properties& p, VectorT& vars )
  {
    vars[Z0] = sqrt(p.rho);
    vars[Z1] = vars[Z0]*(p.u);
    vars[Z2] = vars[Z0]*(p.H);
  }

  /// compute the physical flux
  template < typename FM >
  static void flux( const MODEL::Properties& p,
                    FM& flux)
  {
    throw common::NotImplemented(FromHere(), "flux not implemented for Roe1D");
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv)
  {
    throw common::NotImplemented(FromHere(), "flux_jacobian_eigen_values not implemented for Roe1D");
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV, typename OP >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv,
                                         OP& op )

  {
    throw common::NotImplemented(FromHere(), "flux_jacobian_eigen_values not implemented for Roe1D");
  }

  /// decompose the eigen structure of the flux jacobians projected on the gradients
  template < typename GV, typename EM, typename EV >
  static void flux_jacobian_eigen_structure(const MODEL::Properties& p,
                                            const GV& direction,
                                            EM& Rv,
                                            EM& Lv,
                                            EV& Dv)
  {
    throw common::NotImplemented(FromHere(), "flux_jacobian_eigen_structure not implemented for Roe1D");
  }

  /// compute the PDE residual
  template < typename JM, typename RV >
  static void residual(const MODEL::Properties& p,
                       JM         flux_jacob[],
                       RV&        res)
  {
    throw common::NotImplemented(FromHere(), "flux_jacobian_eigen_structure not implemented for Roe1D");
  }

}; // Roe1D

////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // Physics
} // cf3

#endif // cf3_Physics_NavierStokes_Roe1D_hpp
