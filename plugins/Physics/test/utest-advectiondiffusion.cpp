// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::AdvectionDiffusion"

#include <boost/test/unit_test.hpp>


#include "common/Log.hpp"

#include "AdvectionDiffusion/Physics.hpp"
#include "AdvectionDiffusion/State1D.hpp"

using namespace cf3;
using namespace cf3::AdvectionDiffusion;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( AdvectionDiffusion_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( advdiff1d )
{
  AdvectionDiffusion::State1D ad_state;
  boost::shared_ptr<Solver::Physics> p_ptr = ad_state.create_physics();
  Solver::Physics& p = *p_ptr;
  RealVector state(1); state << 5. ;

  ad_state.set_state(state,p);

  RealVector normal(1); normal << 1.;
  RealVector flux(1);
  RealMatrix rv(1,1);
  RealMatrix lv(1,1);
  RealVector ev(1);

  ad_state.compute_flux(p,normal,flux);
  ad_state.compute_fluxjacobian_eigenvalues(p,normal,ev);
  ad_state.compute_fluxjacobian_right_eigenvectors(p,normal,rv);
  ad_state.compute_fluxjacobian_left_eigenvectors(p,normal,lv);

  BOOST_CHECK_EQUAL(flux[0], 5.);
  BOOST_CHECK_EQUAL(ev[0], 1.);
  BOOST_CHECK_EQUAL(rv(0,0), 1.);
  BOOST_CHECK_EQUAL(lv(0,0), 1.);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

