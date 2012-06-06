//#ifndef SPALARTALLMARAS_H
//#define SPALARTALLMARAS_H

//#endif // SPALARTALLMARAS_H


// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_SpalartAllmaras_hpp
#define cf3_UFEM_SpalartAllmaras_hpp

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

/// solver for SpalartAllmaras turbulence model
class UFEM_API SpalartAllmaras : public LSSActionUnsteady
{
public: // functions

  /// Contructor
  /// @param name of the component
  SpalartAllmaras ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "SpalartAllmaras"; }



private:
  /// Update the copy of the physics coefficients when the physical model changes
  void trigger_physical_model();
  
  /// Copy of the coefficients stored in the physics. Needed to construct the equations
  SUPGCoeffs m_coeffs;
  
  /// Coefficients for Model
   Real cb1, cb2, cw1, cw2, cw3, cv1, ct3, ct4, kappa, sigma;
   Real r, g, d, S;

};

} // UFEM
} // cf3


#endif // cf3_UFEM_NavierStokes_hpp
