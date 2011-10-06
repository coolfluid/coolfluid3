// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Physics_NavierStokes_Cons1D_hpp
#define CF_Physics_NavierStokes_Cons1D_hpp

#include <iostream>

#include "Common/StringConversion.hpp"
#include "Math/Defs.hpp"

#include "Physics/Variables.hpp"

#include "NavierStokes1D.hpp"

namespace CF {
namespace Physics {
namespace NavierStokes {

///////////////////////////////////////////////////////////////////////////////////////

class NavierStokes_API Cons1D : public VariablesT<Cons1D> {

public: //typedefs

  typedef NavierStokes1D     MODEL;

  enum { Rho = 0, RhoU = 1, RhoE = 2 };

  typedef boost::shared_ptr<Cons1D> Ptr;
  typedef boost::shared_ptr<Cons1D const> ConstPtr;

public: // functions

  /// constructor
  /// @param name of the component
  Cons1D ( const std::string& name );

  /// virtual destructor
  virtual ~Cons1D();

  /// Get the class name
  static std::string type_name () { return "Cons1D"; }

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
    p.rhoE  = sol[RhoE];

    p.inv_rho = 1. / p.rho;

    p.u   = p.rhou * p.inv_rho;

    p.uu = p.u*p.u;

    p.P = p.gamma_minus_1 * ( p.rhoE - 0.5 * p.rho * p.uu );

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

    const Real RT = p.P * p.inv_rho;    // RT = p/rho

    p.E = p.rhoE * p.inv_rho;           // E = rhoE / rho

    p.H = p.E + RT;                     // H = E + p/rho

    p.a = sqrt( p.gamma * RT );

    p.a2 = p.a * p.a;

    p.Ma = sqrt( p.uu / p.a2 );

    p.T = RT / p.R;

    p.half_gm1_v2 = 0.5 * p.gamma_minus_1 * p.uu;
  }

  template < typename VectorT >
  static void compute_variables ( const MODEL::Properties& p, VectorT& vars )
  {
    vars[Rho]  = p.rho;
    vars[RhoU] = p.rhou;
    vars[RhoE] = p.rhoE;
  }

  /// compute the physical flux
  template < typename FM >
  static void flux( const MODEL::Properties& p,
                    FM& flux)
  {
    flux(0,XX) = p.rhou;              // rho.u
    flux(1,XX) = p.rhou * p.u + p.P;  // rho.u^2 + P
    flux(2,XX) = p.rhou * p.H;        // rho.u.H
  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv)
  {
    const Real um = p.u * direction[XX];

    Dv <<

      um,
      um + p.a,
      um - p.a;

  }

  /// compute the eigen values of the flux jacobians
  template < typename GV, typename EV, typename OP >
  static void flux_jacobian_eigen_values(const MODEL::Properties& p,
                                         const GV& direction,
                                         EV& Dv,
                                         OP& op )

  {
    const Real um = p.u * direction[XX];

    const Real op_um = op(um);

    Dv <<

      op_um,
      op_um + p.a,
      op_um - p.a;
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

    const Real inv_a  = 1. / p.a;
    const Real inv_a2 = inv_a * inv_a;

    const Real um  = p.u * nx;
    const Real am  = p.a * nx;
    const Real aum = p.a * um;
    const Real ra = 0.5 * p.rho * inv_a;

    const Real coeffM2 = p.half_gm1_v2 * inv_a2;
    const Real uDivA = p.gamma_minus_1 * p.u * inv_a;
    const Real gm1_ov_rhoa = p.gamma_minus_1 / (p.rho*p.a);

    // matrix of right eigenvectors (columns)

    Rv <<

      1.,       ra,                ra,
      p.u,      ra*(p.u+am),       ra*(p.u-am),
      0.5*p.uu, ra*(p.H+aum),      ra*(p.H-aum);

    // matrix of left eigenvectors (rows) = Rv.inverse()
    Lv <<

      1.-coeffM2,                         uDivA*inv_a,             -p.gamma_minus_1*inv_a2,
      p.a*p.inv_rho*(coeffM2-um*inv_a),   p.inv_rho*(nx-uDivA),     gm1_ov_rhoa,
      p.a*p.inv_rho*(coeffM2+um*inv_a),  -p.inv_rho*(nx+uDivA),     gm1_ov_rhoa;

    // diagonal matrix of eigen values
    Dv <<

      um,
      um + p.a,
      um - p.a;

  }

  /// compute the PDE residual
  template < typename JM, typename RV >
  static void residual(const MODEL::Properties& p,
                       JM         flux_jacob[],
                       RV&        res)
  {
    throw Common::NotImplemented(FromHere(),"This function is not yet implemented. Please add implementation.");
  }

}; // Cons1D

////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // Physics
} // CF

#endif // CF_Physics_NavierStokes_Cons1D_hpp
