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

  /// Function to compute the flux for linear advection
  template < typename CV, typename SV, typename GXV, typename GYV, typename FV >
  static void flux(const CV& coord,
                   const SV& u,
                   const GXV& dudx,
                   const GYV& dudy,
                   FV& flux )
  {

    flux[0] =  1.0*dudx[0] + 1.0 * dudy[0];
    flux[1] =  1.0*dudx[1] + 0.8 * dudy[1];
  }

  /// Function to compute the operator L(u)
  template < typename CV, typename SV, typename GV, typename LUV >
  static void Lu(const CV& coord,
                 const SV& u,
                 const GV& gradN,
                 LUV& Lu )
  {
    Lu[0] =  1.0*gradN[XX] + 1.0 * gradN[YY];
    Lu[1] =  1.0*gradN[XX] + 0.8 * gradN[YY];
  }

};

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_LinearAdvSys2D_hpp
