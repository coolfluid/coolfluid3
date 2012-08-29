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

#include "SUPG.hpp"

namespace cf3 {

namespace UFEM {

struct SACoeffs
{
  /// Constants
  Real Cb1;
  Real Cw2;
  Real Cw3;
  Real Cv1;
  Real Cv2;
  Real Sigma;
  Real MuLam;
  Real omega;
  Real shat;
  Real Fv1;
  Real Fv2;
  Real Kappa;
  Real D;
  Real min;
  Real nu_t_cell;
  Real one_over_D_squared;
  Real one_over_Kappa;
  Real one_over_shat;
  Real one_over_Kappa_squared;
  Real one_over_KappaD_squared;
  Real chi;
};

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

  /// Ensure the automatic creation of initial conditions
  virtual void on_initial_conditions_set(InitialConditions& initial_conditions);

  SACoeffs m_sa_coeffs;

  /// Coefficients for Model
   Real cb1, cb2, cw1, cw2, cw3, cv1, one_over_sigma;
   Real r, g, shat;
   Real tau_su;

};

} // UFEM
} // cf3

#endif // cf3_UFEM_NavierStokes_hpp
