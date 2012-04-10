//#ifndef SCALARADVECTION_H
//#define SCALARADVECTION_H

//#endif // SCALARADVECTION_H


// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_ScalarAdvection_hpp
#define cf3_UFEM_ScalarAdvection_hpp

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

#include "NavierStokesOps.hpp"

namespace cf3 {

namespace UFEM {

/// solver for scalar transport
class UFEM_API ScalarAdvection : public LSSActionUnsteady
{
public: // functions

  /// Contructor
  /// @param name of the component
  ScalarAdvection ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "ScalarAdvection"; }

private:
  /// Update the copy of the physics coefficients when the physical model changes
  void trigger_physical_model();
  
  /// Copy of the coefficients stored in the physics. Needed to construct the equations
  SUPGCoeffs m_coeffs;
  
  /// Scalar diffusivity
  Real m_alpha;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_NavierStokes_hpp
