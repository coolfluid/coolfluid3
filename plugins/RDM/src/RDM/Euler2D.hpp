// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Euler2D_hpp
#define CF_Solver_Euler2D_hpp

#include "Common/StringConversion.hpp"
#include "Math/MatrixTypes.hpp"
#include "Mesh/Types.hpp"

#include "RDM/LibRDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

class RDM_API Euler2D {

public: // functions

  /// Constructor
  Euler2D ( );
  /// Destructor
  ~Euler2D();

  /// Get the class name
  static std::string type_name () { return "Euler2D"; }

  /// Number of equations in this physical model
  static const Uint nb_eqs = 4u;

  /// @returns the number of equations
  Uint nbeqs() const { return nb_eqs; }

  template < typename CV, typename SV, typename GV, typename EM, typename EV >
  static void jacobian_eigen_structure(const CV& coord,
                                       const SV& sol,
                                       const GV& gradN,
                                       EM& Rv,
                                       EM& Lv,
                                       EV& Dv)
  {
    const Real gamma = 1.4;                 // diatomic ideal gas
    const Real gamma_minus_1 = gamma - 1.;
//    const Real R = 287.058;                 // air

    const Real nx = gradN[XX];
    const Real ny = gradN[YY];

    const Real rho   = sol[0];
    const Real rhoE  = sol[3];

    const Real inv_rho = 1. / rho;

    const Real u   = sol[1] * inv_rho;
    const Real v   = sol[2] * inv_rho;

    const Real uuvv = u*u + v*v;

    const Real H = gamma*rhoE/rho - 0.5*gamma_minus_1*uuvv;

    const Real a2 = gamma_minus_1*(H - 0.5*uuvv);

    if( a2 <= 0 )
      throw Common::BadValue( FromHere(), "Speed of sound negative at coordinates ["
                                   + Common::to_str(coord[XX]) + ","
                                   + Common::to_str(coord[YY])
                                   + "]");

//    const Real T = a2 / ( gamma * R );
//    const Real P = rho*R*T;
//    const Real E = H - P/rho;

    const Real a      = sqrt(a2);
    const Real inv_a  = 1./a;
    const Real inv_a2 = inv_a*inv_a;

    const Real um = u*nx + v*ny;
    const Real ra = 0.5*rho*inv_a;

    const Real coeffM2 = 0.5*gamma_minus_1*(u*u + v*v)*inv_a2;
    const Real uDivA = gamma_minus_1*u*inv_a;
    const Real vDivA = gamma_minus_1*v*inv_a;
    const Real rho_a = rho*a;

    const Real gm1_ov_rhoa = gamma_minus_1/rho_a;

    // matrix of right eigen vectors R

    Rv(0,0) = 1.;
    Rv(0,1) = 0.;
    Rv(0,2) = ra;
    Rv(0,3) = ra;
    Rv(1,0) = u;
    Rv(1,1) = rho*ny;
    Rv(1,2) = ra*(u + a*nx);
    Rv(1,3) = ra*(u - a*nx);
    Rv(2,0) = v;
    Rv(2,1) = -rho*nx;;
    Rv(2,2) = ra*(v + a*ny);
    Rv(2,3) = ra*(v - a*ny);
    Rv(3,0) = 0.5*(u*u +v*v);
    Rv(3,1) = rho*(u*ny - v*nx);
    Rv(3,2) = ra*(H + a*um);
    Rv(3,3) = ra*(H - a*um);

    // matrix of left eigen vectors L = R.inverse();

    Lv(0,0) = 1.- coeffM2;
    Lv(0,1) = uDivA*inv_a;
    Lv(0,2) = vDivA*inv_a;
    Lv(0,3) = -gamma_minus_1*inv_a2;
    Lv(1,0) = inv_rho*(v*nx - u*ny);
    Lv(1,1) = inv_rho*ny;
    Lv(1,2) = -inv_rho*nx;
    Lv(1,3) = 0.0;
    Lv(2,0) = a*inv_rho*(coeffM2 - um*inv_a);
    Lv(2,1) = inv_rho*(nx - uDivA);
    Lv(2,2) = inv_rho*(ny - vDivA);
    Lv(2,3) = gm1_ov_rhoa;
    Lv(3,0) = a*inv_rho*(coeffM2 + um*inv_a);
    Lv(3,1) = -inv_rho*(nx + uDivA);
    Lv(3,2) = -inv_rho*(ny + vDivA);
    Lv(3,3) = gm1_ov_rhoa;

    // diagonal matrix of eigen values

    Dv[0] = um;
    Dv[1] = um;
    Dv[2] = um + a;
    Dv[3] = um - a;

  }

  template < typename CV, typename SV, typename GXV, typename GYV, typename JM, typename LUV >
  static void Lu(const CV&  coord,
                 const SV&  sol,
                 const GXV& dudx,
                 const GYV& dudy,
                 JM         flux_jacob[],
                 LUV&       Lu)
  {
    const Real gamma = 1.4;
    const Real gamma_minus_1 = gamma - 1.;
    const Real gamma_minus_3 = gamma - 3.;

    const Real rho   = sol[0];
    const Real rhoE  = sol[3];

    const Real inv_rho = 1. / rho;
    const Real u   = sol[1] * inv_rho;
    const Real v   = sol[2] * inv_rho;

    const Real uuvv = u*u + v*v;

    const Real H = gamma*rhoE*inv_rho - 0.5*gamma_minus_1*uuvv;

    const Real uu = u*u;
    const Real uv = u*v;
    const Real vv = v*v;

    const Real sum_v2 = 0.5*gamma_minus_1*(uu + vv);

    JM& A = flux_jacob[XX];

  //    A.setZero(); // assume are zeroed

    A(0,1) = 1.;
    A(1,0) = sum_v2 - uu;
    A(1,1) = -gamma_minus_3*u;
    A(1,2) = -gamma_minus_1*v;
    A(1,3) = gamma_minus_1;
    A(2,0) = -uv;
    A(2,1) = v;
    A(2,2) = u;
    A(3,0) = sum_v2*u - u*H;
    A(3,1) = -gamma_minus_1*uu + H;
    A(3,2) = -gamma_minus_1*uv;
    A(3,3) = gamma*u;

    JM& B = flux_jacob[YY];

    //    B.setZero(); // assume are zeroed

    B(0,2) = 1.;
    B(1,0) = -uv;
    B(1,1) = v;
    B(1,2) = u;
    B(2,0) = sum_v2 - vv;
    B(2,1) = -gamma_minus_1*u;
    B(2,2) = -gamma_minus_3*v;
    B(2,3) = gamma_minus_1;
    B(3,0) = sum_v2*v - v*H;
    B(3,1) = -gamma_minus_1*uv;
    B(3,2) = -gamma_minus_1*vv + H;
    B(3,3) = gamma*v;

    Lu = A * dudx + B * dudy;
  }

}; // Euler2D

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_Euler2D_hpp
