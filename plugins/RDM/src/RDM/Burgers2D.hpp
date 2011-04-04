// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Burgers2D_hpp
#define CF_Solver_Burgers2D_hpp

#include "Math/MatrixTypes.hpp"

#include "RDM/LibRDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

class RDM_API Burgers2D {

public: // functions
  /// Constructor
  Burgers2D ( );

  /// Destructor
  ~Burgers2D();

  /// Get the class name
  static std::string type_name () { return "Burgers2D"; }

  /// Number of equations in this physical model
  static const Uint nb_eqs = 1u;

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

    flux[0] = u[0]*dudx[0] + dudy[0];
  }

  /// Function to compute the operator L(u)
  template < typename CV, typename SV, typename GV, typename LUV >
  static void Lu(const CV& coord,
                 const SV& u,
                 const GV& gradN,
                 LUV& Lu )
  {
    Lu[0] = u[0]*gradN[XX] + gradN[YY];
  }

};

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_RotationAdv2D_hpp
