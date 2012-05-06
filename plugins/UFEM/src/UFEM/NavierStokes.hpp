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
#include "LSSActionUnsteady.hpp"
#include "NavierStokesPhysics.hpp"

namespace cf3 {

namespace UFEM {

/// solver for the unsteady incompressible Navier-Stokes equations
class UFEM_API NavierStokes : public LSSActionUnsteady
{
public: // functions

  /// Contructor
  /// @param name of the component
  NavierStokes ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "NavierStokes"; }
  
private:
  /// Update the copy of the physics coefficients when the physical model changes
  void trigger_physical_model();
  
  /// Create the solver structure, based on the choice of specialized code
  void trigger_use_specializations();
  
  /// Copy of the coefficients stored in the physics. Needed to construct the equations
  SUPGCoeffs m_coeffs;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_NavierStokes_hpp
