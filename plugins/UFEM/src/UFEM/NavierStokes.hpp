// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_NavierStokes_hpp
#define cf3_UFEM_NavierStokes_hpp

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/max.hpp>

#define BOOST_PROTO_MAX_ARITY 10
#ifdef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
  #undef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
#endif
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10

#include <boost/scoped_ptr.hpp>

#include "LibUFEM.hpp"
#include "LinearSolverUnsteady.hpp"
#include "NavierStokesOps.hpp"

namespace cf3 {

namespace UFEM {

/// solver for the unsteady incompressible Navier-Stokes equations
class UFEM_API NavierStokes : public LinearSolverUnsteady
{
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
} // cf3


#endif // cf3_UFEM_NavierStokes_hpp
