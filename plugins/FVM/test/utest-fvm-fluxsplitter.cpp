// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::FVM"

#include <boost/test/unit_test.hpp>

#include "Common/CreateComponent.hpp"
#include "Common/Log.hpp"

#include "FVM/RoeFluxSplitter.hpp"

using namespace CF;
using namespace CF::FVM;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( FVM_FluxSplitter_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Roe )
{
  RoeFluxSplitter roe("roe");
  
  RealVector left(3);
  RealVector right(3);
  
  Real g=1.4;
  
  const Real r_L = 4.696;     const Real r_R = 1.408;
  const Real u_L = 0.;        const Real u_R = 0.;
  const Real p_L = 404400;    const Real p_R = 101100;

  left <<  r_L, r_L*u_L, p_L/(g-1.) + 0.5*r_L*u_L*u_L;
  right << r_R, r_R*u_R, p_R/(g-1.) + 0.5*r_R*u_R*u_R;

  RealVector F_L(3); F_L << 0., 404400, 0.;
  RealVector F_R(3); F_R << 0., 101100, 0.;
  BOOST_CHECK_EQUAL(roe.flux(left),F_L);
  BOOST_CHECK_EQUAL(roe.flux(right),F_R);
  const Real tol (0.000001);
  BOOST_CHECK_CLOSE(roe.solve(left,right)[0] , 450.190834 , tol);
  BOOST_CHECK_CLOSE(roe.solve(left,right)[1] , 252750 , tol);
  BOOST_CHECK_CLOSE(roe.solve(left,right)[2] , 127710965.918678 , tol);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

