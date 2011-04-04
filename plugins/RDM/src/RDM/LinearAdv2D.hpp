// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_LinearAdv2D_hpp
#define CF_Solver_LinearAdv2D_hpp

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
    flux[0] =  1.0 * dudx[0] + 1.0 * dudy[0];
  }

  /// Function to compute the operator L(u)
  template < typename CV, typename SV, typename GV, typename LUV >
  static void Lu(const CV& coord,
                 const SV& u,
                 const GV& gradN,
                 LUV& Lu )
  {
    Lu[0] =  1.0 * gradN[XX] + 1.0 * gradN[YY];
  }

};

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_LinearAdv2D_hpp
