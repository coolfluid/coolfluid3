// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_LinearAdvSys2D_hpp
#define CF_RDM_LinearAdvSys2D_hpp

#include "Math/MatrixTypes.hpp"
#include "Mesh/Types.hpp"

#include "RDM/LibRDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

class RDM_API LinearAdvSys2D {

public: // functions

  /// Constructor
  LinearAdvSys2D ( );
  /// Destructor
  ~LinearAdvSys2D();

  /// Get the class name
  static std::string type_name () { return "LinearAdvSys2D"; }

  /// Number of equations in this physical model
  static const Uint neqs = 2u;
  /// Number of dimensions of this physical model
  static const Uint ndim = 2u;

  /// @returns the number of equations
  Uint nbeqs() const { return neqs; }
  /// @returns the number of equations
  Uint dim() const { return ndim; }

  /// precomputed physical properties more less common to functions of this physical model
  struct Properties
  {
    Properties()
    {
      Vx[0] = 1.0;
      Vx[1] = 1.0;
      Vy[0] = 1.0;
      Vy[1] = 0.8;
    }

    Real Vx [neqs];
    Real Vy [neqs];
  };

  /// compute physical properties
  template < typename CV, typename SV, typename GXV, typename GYV >
  static void compute_properties ( const CV&  coord,
                                   const SV&  sol,
                                   const GXV& dudx,
                                   const GYV& dudy,
                                   Properties& p )
  {
  }

  /// compute the eigen values of the flux jacobians
  template < typename CV, typename SV, typename GV, typename EV >
  static void jacobian_eigen_values( const Properties& p,
                                     const CV& coord,
                                     const SV& sol,
                                     const GV& gradN,
                                     EV& Dv)
  {
    Dv[0]   = p.Vx[0] * gradN[XX] + p.Vy[0] * gradN[YY];
    Dv[1]   = p.Vx[1] * gradN[XX] + p.Vy[1] * gradN[YY];
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
    Rv(0,0) = 1.;
    Rv(0,1) = 0.;
    Rv(1,0) = 0.;
    Rv(1,1) = 1.;

    Lv(0,0) = 1.;
    Lv(0,1) = 0.;
    Lv(1,0) = 0.;
    Lv(1,1) = 1.;

    Dv[0]   = p.Vx[0] * gradN[XX] + p.Vy[0] * gradN[YY];
    Dv[1]   = p.Vx[1] * gradN[XX] + p.Vy[1] * gradN[YY];
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
    JM& A = flux_jacob[XX];
    JM& B = flux_jacob[YY];

    A(0,0) = p.Vx[0];
    A(1,1) = p.Vx[1];

    B(0,0) = p.Vy[0];
    B(1,1) = p.Vy[1];

    Lu = A * dudx + B * dudy;
  }

};

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_LinearAdvSys2D_hpp
