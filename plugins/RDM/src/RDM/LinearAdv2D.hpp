// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_LinearAdv2D_hpp
#define CF_RDM_LinearAdv2D_hpp

#include "RDM/LibRDM.hpp"
#include "Math/MatrixTypes.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

class RDM_API LinearAdv2D
{

public: // functions
  /// Constructor
  LinearAdv2D ( );

  /// Destructor
  ~LinearAdv2D();

  /// Get the class name
  static std::string type_name () { return "LinearAdv2D"; }

  /// Number of equations in this physical model
  static const Uint neqs = 1u;
  /// Number of dimensions of this physical model
  static const Uint ndim = 2u;

  /// @returns the number of equations
  Uint nbeqs() const { return neqs; }
  /// @returns the number of equations
  Uint dim() const { return ndim; }

  /// precomputed physical properties more less common to functions of this physical model
  struct Properties
  {
    Properties() : Vx(1.0) , Vy(1.0) {}

    const Real Vx;
    const Real Vy;
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
    Dv[0]   = p.Vx * gradN[XX] + p.Vy * gradN[YY];
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
    Lv(0,0) = 1.;
    Dv[0]   = p.Vx * gradN[XX] + p.Vy * gradN[YY];
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

    A(0,0) = p.Vx;
    B(0,0) = p.Vy;

    Lu = A * dudx + B * dudy;
  }

};

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_LinearAdv2D_hpp
