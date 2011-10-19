// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_NavierStokes_hpp
#define CF_UFEM_NavierStokes_hpp

#include <boost/scoped_ptr.hpp>

#include "LibUFEM.hpp"
#include "LinearSolverUnsteady.hpp"
#include "NavierStokesOps.hpp"

namespace CF {

namespace UFEM {

/// Solver for the incompressible Navier-Stokes equations
class UFEM_API NavierStokes : public LinearSolverUnsteady
{
public: // typedefs

  typedef boost::shared_ptr<NavierStokes> Ptr;
  typedef boost::shared_ptr<NavierStokes const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  NavierStokes ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "NavierStokes"; }

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
};

} // UFEM
} // CF


#endif // CF_UFEM_NavierStokes_hpp
