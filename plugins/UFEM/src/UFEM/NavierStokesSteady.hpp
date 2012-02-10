// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_NavierStokesSteady_hpp
#define cf3_UFEM_NavierStokesSteady_hpp

#include <boost/scoped_ptr.hpp>

#include "LibUFEM.hpp"
#include "LinearSolver.hpp"
#include "NavierStokesOps.hpp"

namespace cf3 {

namespace UFEM {

/// Solver for the steady incompressible Navier-Stokes equations
class UFEM_API NavierStokesSteady : public LinearSolver
{
public: // functions

  /// Contructor
  /// @param name of the component
  NavierStokesSteady ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "NavierStokesSteady"; }
  
private:
  
  /// Storage for stabilization coefficients
  SUPGCoeffs m_coeffs;

  /// Initial condition for p
  Real m_p0;

  /// Initial condition for v
  RealVector m_u0;

  /// Trigger for rho
  void trigger_rho();

  /// Trigger on initial condition for velocity
  void trigger_u();
  
  Real m_p_under_relaxation;
  Real m_u_under_relaxation;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_NavierStokesSteady_hpp
