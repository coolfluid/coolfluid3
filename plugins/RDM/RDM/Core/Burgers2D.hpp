// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_Burgers2D_hpp
#define CF_RDM_Burgers2D_hpp

#include "Math/MatrixTypes.hpp"

#include "RDM/Core/LibCore.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

class RDM_CORE_API Burgers2D {

public: // functions
  /// Constructor
  Burgers2D ( );

  /// Destructor
  ~Burgers2D();

  /// Get the class name
  static std::string type_name () { return "Burgers2D"; }

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
    Real Vx;
    Real Vy;
  };

  /// compute physical properties
  template < typename CV, typename SV, typename GM >
  static void compute_properties ( const CV&  coord,
                                   const SV&  sol,
                                   const GM& gradu,
                                   Properties& p )
  {
    p.Vx = sol[0];
    p.Vy = 1.0;
  }

  /// compute the physical flux
  template < typename CV, typename SV, typename FM >
  static void flux( const Properties& p,
                    const CV& coord,
                    const SV& sol,
                    FM& flux)
  {
    flux(0,XX)   = p.Vx * sol[0];
    flux(0,YY)   = p.Vy * sol[0];
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
  template < typename CV, typename SV, typename GM, typename JM, typename LUV >
  static void Lu(const Properties& p,
                 const CV&  coord,
                 const SV&  sol,
                 const GM& gradu,
                 JM         flux_jacob[],
                 LUV&       Lu)
  {
    JM& A = flux_jacob[XX];
    JM& B = flux_jacob[YY];

    A(0,0) = p.Vx;
    B(0,0) = p.Vy;

    Lu = A * gradu[XX]+ B * gradu[YY];
  }

};

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_RotationAdv2D_hpp
