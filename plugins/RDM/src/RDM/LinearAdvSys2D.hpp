// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_LinearAdvSys2D_hpp
#define CF_Solver_LinearAdvSys2D_hpp

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
  static const Uint nb_eqs = 2u;

  /// @returns the number of equations
  Uint nbeqs() const { return nb_eqs; }

  /// decompose the eigen structure of the flux jacobians projected on the gradients
  template < typename CV, typename SV, typename GV, typename EM, typename EV >
  static void jacobian_eigen_structure(const CV& coord,
                                       const SV& sol,
                                       const GV& gradN,
                                       EM& Rv,
                                       EM& Lv,
                                       EV& Dv)
  {
    Rv(0,0) = 1.;
    Rv(0,1) = 0.;
    Rv(1,1) = 1.;
    Rv(1,0) = 0.;

    Lv(0,0) = 1.;
    Lv(0,1) = 0.;
    Lv(1,1) = 1.;
    Lv(1,0) = 0.;

    Dv[0]   = 1.0 * gradN[XX] + 1.0 * gradN[YY];
    Dv[1]   = 1.0 * gradN[XX] + 0.8 * gradN[YY];
  }

  /// compute the PDE residual
  template < typename CV, typename SV, typename GXV, typename GYV, typename JM, typename LUV >
  static void Lu(const CV&  coord,
                 const SV&  sol,
                 const GXV& dudx,
                 const GYV& dudy,
                 JM         flux_jacob[],
                 LUV&       Lu)
  {
    JM& A = flux_jacob[XX];
    JM& B = flux_jacob[YY];

    A(0,0) = 1.0;
    A(1,1) = 1.0;

    B(0,0) = 1.0;
    B(1,1) = 0.8;

    Lu = A * dudx + B * dudy;
  }

};

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_LinearAdvSys2D_hpp
