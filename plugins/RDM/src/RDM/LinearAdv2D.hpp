// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_LinearAdv2D_hpp
#define CF_Solver_LinearAdv2D_hpp

//#include <Eigen/Dense>
#include "Mesh/LibMesh.hpp"
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
  static Uint nbeqs() { return 1u; }

  /// Function to compute the variable beta
  static Real beta(const RealVector2 & coord, const Real sf, const RealVector2 & grad_sf);

  /// Compute the operator applied to solution
  static Real residual(const RealVector2 & coord, const Real sol, const RealVector2 & grad_sol);
};

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_LinearAdv2D_hpp
