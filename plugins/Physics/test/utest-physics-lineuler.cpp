// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::Euler"

#include <iostream>
#include <boost/test/unit_test.hpp>

#include "cf3/common/Log.hpp"
#include "cf3/common/Core.hpp"
#include "cf3/common/Environment.hpp"
#include "cf3/physics/lineuler/lineuler2d/Functions.hpp"

using namespace std;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::physics::lineuler::lineuler2d;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( Euler_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Test_LinEuler2d_convection )
{
  Data p;
  p.gamma=4.;
  p.U0 << 0.5, 0.;
  p.rho0 = 1.;
  p.p0 = 1.;
  p.c0 = std::sqrt(p.gamma*p.p0/p.rho0);
  
  RowVector_NEQS prim;
  prim << 0.1, 0.2, 0.3, 0.4;
  p.compute_from_primitive(prim);
  
  ColVector_NDIM normal; normal << 1., 1.;
  RowVector_NEQS flux;
  Real wave_speed;
  compute_convective_flux( p, normal, flux , wave_speed );
  
  BOOST_CHECK_EQUAL( flux[0] , 0.55 );
  BOOST_CHECK_EQUAL( flux[1] , 0.5  );
  BOOST_CHECK_EQUAL( flux[2] , 0.55 );
  BOOST_CHECK_EQUAL( flux[3] , 2.2  );
  BOOST_CHECK_EQUAL( wave_speed, 2.5 );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Test_LinEuler2d_riemann )
{
  Data pL, pR;
  
  pL.gamma=4.;                               pR.gamma = pL.gamma;
  pL.U0 << 0.5, 0.;                          pR.U0    = pL.U0;
  pL.rho0 = 1.;                              pR.rho0  = pL.rho0;
  pL.p0 = 1.;                                pR.p0    = pL.p0;
  pL.c0 = std::sqrt(pL.gamma*pL.p0/pL.rho0); pR.c0    = pL.c0;
  pL.cons << 0.1, 0.2, 0.3, 0.4;             pR.cons  = pL.cons*2.;

  RowVector_NEQS flux_pos, flux_neg;
  ColVector_NDIM normal; normal << 1.,1.;
  Real wave_speed;
  
  compute_rusanov_flux( pL, pR, normal, flux_pos , wave_speed );
  std::cout << wave_speed << "\t" << flux_pos << std::endl;

  compute_rusanov_flux( pR, pL, -normal, flux_neg , wave_speed );
  std::cout << wave_speed << "\t" << flux_neg << std::endl;

  BOOST_CHECK_EQUAL ( flux_pos, -flux_neg );

  compute_cir_flux( pL, pR,  normal, flux_pos , wave_speed );
  std::cout << wave_speed << "\t" << flux_pos << std::endl;

  compute_cir_flux( pR, pL, -normal, flux_neg , wave_speed );
  std::cout << wave_speed << "\t" << flux_neg << std::endl;
  
  BOOST_CHECK_EQUAL ( flux_pos, -flux_neg );
  
  
  Matrix_NEQSxNEQS A, R, L;
  RowVector_NEQS D;
  compute_absolute_flux_jacobian(pL, normal, A);
  compute_convective_left_eigenvectors(pL, normal, L);
  compute_convective_right_eigenvectors(pL, normal, R);
  compute_convective_eigenvalues(pL, normal, D);
  
  Matrix_NEQSxNEQS Acheck;
  Acheck <<
    0.5,  0.25,  0.25, 0.375,
    0,    2.5,   1.5,  0.25,
    0,    1.5,   2.5,  0.25,
    0,    1,     1,    2;
  BOOST_CHECK( A == Acheck );
  BOOST_CHECK( R*D.cwiseAbs().asDiagonal()*L == Acheck );
  
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

