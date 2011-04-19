// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_Euler2D_hpp
#define CF_RDM_Euler2D_hpp

#include "Common/StringConversion.hpp"
#include "Math/MatrixTypes.hpp"
#include "Mesh/Types.hpp"

#include "RDM/Core/LibCore.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

class RDM_CORE_API Euler2D {

public: // functions

  /// Constructor
  Euler2D ( );
  /// Destructor
  ~Euler2D();

  /// Get the class name
  static std::string type_name () { return "Euler2D"; }

  /// Number of equations in this physical model
  static const Uint neqs = 4u;
  /// Number of dimensions of this physical model
  static const Uint ndim = 2u;

  /// @returns the number of equations
  Uint nbeqs() const { return neqs; }
  /// @returns the number of equations
  Uint dim() const { return ndim; }

  /// precomputed physical properties more less common to functions of this physical model
  struct Properties
  {
    Real gamma;
    Real gamma_minus_1;
    Real R;

    Real rho;
    Real inv_rho;
    Real rhoE;
    Real u;
    Real v;
    Real uuvv;
    Real H;
    Real a2;
    Real a;
    Real P;
    Real T;
    Real E;
    Real half_gm1_v2;
  };

  /// compute physical properties
  template < typename CV, typename SV, typename GXV, typename GYV >
  static void compute_properties ( const CV&  coord,
                                   const SV&  sol,
                                   const GXV& dudx,
                                   const GYV& dudy,
                                   Properties& p )
  {
    p.gamma = 1.4;                 // diatomic ideal gas
    p.gamma_minus_1 = p.gamma - 1.;
    p.R = 287.058;                 // air

    p.rho   = sol[0];
    p.rhoE  = sol[3];

    p.inv_rho = 1. / p.rho;

    p.u   = sol[1] * p.inv_rho;
    p.v   = sol[2] * p.inv_rho;

    p.uuvv = p.u*p.u + p.v*p.v;

    p.H = p.gamma * p.rhoE / p.rho - 0.5 * p.gamma_minus_1 * p.uuvv;

    p.a2 = p.gamma_minus_1 * ( p.H - 0.5 * p.uuvv);

    if( p.a2 <= 0 )
      throw Common::BadValue( FromHere(), "Speed of sound negative at coordinates ["
                                   + Common::to_str(coord[XX]) + ","
                                   + Common::to_str(coord[YY])
                                   + "]");

    p.a = sqrt( p.a2 );

//    p.T = p.a2 / ( p.gamma * p.R );

    p.P = p.rho * p.R * p.T;
    p.E = p.H - p.P * p.inv_rho;

    p.half_gm1_v2 = 0.5 * p.gamma_minus_1 * p.uuvv;

  }

  /// compute the eigen values of the flux jacobians
  template < typename CV, typename SV, typename GV, typename EV >
  static void jacobian_eigen_values( const Properties& p,
                                     const CV& coord,
                                     const SV& sol,
                                     const GV& gradN,
                                     EV& Dv)
  {
    const Real nx = gradN[XX];
    const Real ny = gradN[YY];

    const Real um = p.u * nx + p.v * ny;

    Dv[0] = um;
    Dv[1] = um;
    Dv[2] = um + p.a;
    Dv[3] = um - p.a;
  }

  /// decompose the eigen structure of the flux jacobians projected on the gradients
  template < typename CV, typename SV, typename GV, typename EM, typename EV >
  static void jacobian_eigen_structure(const Properties& p,
                                       const CV& coord,
                                       const SV& sol,
                                       const GV& gradN,
                                       EM& Rv,
                                       EM& Lv,
                                       EV& Dv)
  {
    const Real nx = gradN[XX];
    const Real ny = gradN[YY];

    const Real inv_a  = 1. / p.a;
    const Real inv_a2 = inv_a * inv_a;

    const Real um = p.u * nx + p.v * ny;
    const Real ra = 0.5 * p.rho * inv_a;

    const Real coeffM2 = p.half_gm1_v2 * inv_a2;
    const Real uDivA = p.gamma_minus_1 * p.u * inv_a;
    const Real vDivA = p.gamma_minus_1 * p.v * inv_a;
    const Real rho_a = p.rho * p.a;

    const Real gm1_ov_rhoa = p.gamma_minus_1 / rho_a;

    // matrix of right eigen vectors R

    Rv(0,0) = 1.;
    Rv(0,1) = 0.;
    Rv(0,2) = ra;
    Rv(0,3) = ra;
    Rv(1,0) = p.u;
    Rv(1,1) = p.rho * ny;
    Rv(1,2) = ra*(p.u + p.a*nx);
    Rv(1,3) = ra*(p.u - p.a*nx);
    Rv(2,0) = p.v;
    Rv(2,1) = -p.rho*nx;;
    Rv(2,2) = ra*(p.v + p.a*ny);
    Rv(2,3) = ra*(p.v - p.a*ny);
    Rv(3,0) = 0.5 * p.uuvv;
    Rv(3,1) = p.rho * (p.u*ny - p.v*nx);
    Rv(3,2) = ra*(p.H + p.a*um);
    Rv(3,3) = ra*(p.H - p.a*um);

    // matrix of left eigen vectors L = R.inverse();

    Lv(0,0) = 1.- coeffM2;
    Lv(0,1) = uDivA*inv_a;
    Lv(0,2) = vDivA*inv_a;
    Lv(0,3) = -p.gamma_minus_1 * inv_a2;
    Lv(1,0) = p.inv_rho * (p.v*nx - p.u*ny);
    Lv(1,1) = p.inv_rho * ny;
    Lv(1,2) = -p.inv_rho * nx;
    Lv(1,3) = 0.0;
    Lv(2,0) = p.a*p.inv_rho * (coeffM2 - um*inv_a);
    Lv(2,1) = p.inv_rho * (nx - uDivA);
    Lv(2,2) = p.inv_rho * (ny - vDivA);
    Lv(2,3) = gm1_ov_rhoa;
    Lv(3,0) = p.a*p.inv_rho*(coeffM2 + um*inv_a);
    Lv(3,1) = -p.inv_rho*(nx + uDivA);
    Lv(3,2) = -p.inv_rho*(ny + vDivA);
    Lv(3,3) = gm1_ov_rhoa;

    // diagonal matrix of eigen values

    Dv[0] = um;
    Dv[1] = um;
    Dv[2] = um + p.a;
    Dv[3] = um - p.a;

  }

  /// compute the PDE residual
  template < typename CV, typename SV, typename GXV, typename GYV, typename JM, typename LUV >
  static void Lu(const Properties& p,
                 const CV&  coord,
                 const SV&  sol,
                 const GXV& dudx,
                 const GYV& dudy,
                 JM         flux_jacob[],
                 LUV&       Lu)
  {
    const Real gamma_minus_3 = p.gamma - 3.;

    const Real uu = p.u * p.u;
    const Real uv = p.u * p.v;
    const Real vv = p.v * p.v;

    JM& A = flux_jacob[XX];

  //    A.setZero(); // assume are zeroed

    A(0,1) = 1.;
    A(1,0) = p.half_gm1_v2 - uu;
    A(1,1) = -gamma_minus_3*p.u;
    A(1,2) = -p.gamma_minus_1*p.v;
    A(1,3) = p.gamma_minus_1;
    A(2,0) = -uv;
    A(2,1) = p.v;
    A(2,2) = p.u;
    A(3,0) = p.half_gm1_v2*p.u - p.u * p.H;
    A(3,1) = -p.gamma_minus_1*uu + p.H;
    A(3,2) = -p.gamma_minus_1*uv;
    A(3,3) = p.gamma*p.u;

    JM& B = flux_jacob[YY];

    //    B.setZero(); // assume are zeroed

    B(0,2) = 1.;
    B(1,0) = -uv;
    B(1,1) = p.v;
    B(1,2) = p.u;
    B(2,0) = p.half_gm1_v2 - vv;
    B(2,1) = -p.gamma_minus_1*p.u;
    B(2,2) = -gamma_minus_3*p.v;
    B(2,3) = p.gamma_minus_1;
    B(3,0) = p.half_gm1_v2*p.v - p.v*p.H;
    B(3,1) = -p.gamma_minus_1*uv;
    B(3,2) = -p.gamma_minus_1*vv + p.H;
    B(3,3) = p.gamma*p.v;

    Lu = A * dudx + B * dudy;
  }

}; // Euler2D

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_Euler2D_hpp
