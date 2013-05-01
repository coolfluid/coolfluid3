// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::Euler"

#include <iostream>
#include <boost/test/unit_test.hpp>

#include "math/Defs.hpp"

#include "cf3/common/Log.hpp"
#include "cf3/common/Core.hpp"
#include "cf3/common/Environment.hpp"
#include "cf3/physics/euler/euler1d/Functions.hpp"
#include "cf3/physics/euler/euler2d/Functions.hpp"

using namespace std;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::physics::euler;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( Euler_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Test_Euler1D_convection )
{
  euler1d::Data p;
  p.gamma=1.4;
  p.R=287.05;
  
  euler1d::RowVector_NEQS prim;
  prim << 1.225, 30, 101300;
  p.compute_from_primitive(prim);
  
  euler1d::ColVector_NDIM normal; normal << 1.;
  euler1d::RowVector_NEQS flux;
  Real wave_speed;
  compute_convective_flux( p, normal, flux , wave_speed );
  
  BOOST_CHECK_CLOSE( flux[0] , p.rho*p.u           , 1e-10);
  BOOST_CHECK_CLOSE( flux[1] , p.rho*p.u*p.u + p.p , 1e-10);
  BOOST_CHECK_CLOSE( flux[2] , (p.rho*p.E+p.p)*p.u , 1e-6);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Test_Euler1D_riemann )
{
  euler1d::Data pL, pR;
  
  pL.gamma=1.4;                                      pR.gamma=1.4;
  pL.R=287.05;                                       pR.R=287.05;

  euler1d::RowVector_NEQS prim_left, prim_right;
  prim_left  << 4.696, 0, 404400; pL.compute_from_primitive(prim_left);
  prim_right << 1.408, 0, 101100; pR.compute_from_primitive(prim_right);

  BOOST_CHECK_EQUAL(pL.rho,4.696);
  BOOST_CHECK_EQUAL(pL.u,0);
  BOOST_CHECK_EQUAL(pL.p,404400);
  BOOST_CHECK_CLOSE(pL.c2, (pL.gamma-1.)*(pL.H-0.5*pL.u2),1e-8);
  euler1d::RowVector_NEQS flux_pos, flux_neg;
  euler1d::ColVector_NDIM normal; normal << 1.;
  Real wave_speed;
  
  compute_rusanov_flux( pL, pR, normal, flux_pos , wave_speed );
  std::cout << flux_pos << std::endl;

  compute_rusanov_flux( pR, pL, -normal, flux_neg , wave_speed );
  std::cout << flux_neg << std::endl;

  BOOST_CHECK_EQUAL ( flux_pos, -flux_neg );

  compute_roe_flux( pL, pR,  normal, flux_pos , wave_speed );
  std::cout << flux_pos << std::endl;

  compute_roe_flux( pR, pL, -normal, flux_neg , wave_speed );
  std::cout << flux_neg << std::endl;
  
  BOOST_CHECK_EQUAL ( flux_pos, -flux_neg );

  compute_hlle_flux( pL, pR,  normal, flux_pos , wave_speed );
  std::cout << flux_pos << std::endl;

  compute_hlle_flux( pR, pL, -normal, flux_neg , wave_speed );
  std::cout << flux_neg << std::endl;

  BOOST_CHECK_EQUAL ( flux_pos, -flux_neg );

}



BOOST_AUTO_TEST_CASE( Test_Euler2D_convection )
{
  euler2d::Data p;
  p.gamma=1.4;
  p.R=287.05;

  euler2d::RowVector_NEQS prim;
  prim << 1.225, 30, 30, 101300;
  p.compute_from_primitive(prim);

  euler2d::ColVector_NDIM normal; normal << 1., 1.; normal.normalize();
  euler2d::RowVector_NEQS flux;
  Real wave_speed;
  compute_convective_flux( p, normal, flux , wave_speed );

  Real un = p.U.dot(normal);
  BOOST_CHECK_CLOSE( flux[0] , p.rho*un            , 1e-10);
  BOOST_CHECK_CLOSE( flux[1] , p.rho*p.U[XX]*un + p.p*normal[XX] , 1e-10);
  BOOST_CHECK_CLOSE( flux[2] , p.rho*p.U[YY]*un + p.p*normal[YY] , 1e-10);
  BOOST_CHECK_CLOSE( flux[3] , (p.rho*p.E+p.p)*un , 1e-6);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Test_Euler2D_riemann )
{
  euler2d::Data pL, pR;

  pL.gamma=1.4;                                      pR.gamma=1.4;
  pL.R=287.05;                                       pR.R=287.05;

  euler2d::RowVector_NEQS prim_left, prim_right;
  prim_left  << 4.696, 0, 0, 404400; pL.compute_from_primitive(prim_left);
  prim_right << 1.408, 0, 0, 101100; pR.compute_from_primitive(prim_right);

  BOOST_CHECK_EQUAL(pL.rho,4.696);
  BOOST_CHECK_EQUAL(pL.U[XX],0);
  BOOST_CHECK_EQUAL(pL.U[YY],0);
  BOOST_CHECK_EQUAL(pL.p,404400);
  BOOST_CHECK_CLOSE(pL.c2, (pL.gamma-1.)*(pL.H-0.5*pL.U2),1e-8);
  euler2d::RowVector_NEQS flux_pos, flux_neg;
  euler2d::ColVector_NDIM normal; normal << 0.,1.; normal.normalize();
  Real wave_speed;

  compute_rusanov_flux( pL, pR, normal, flux_pos , wave_speed );
  std::cout << flux_pos << std::endl;

  compute_rusanov_flux( pR, pL, -normal, flux_neg , wave_speed );
  std::cout << flux_neg << std::endl;

  BOOST_CHECK_EQUAL ( flux_pos, -flux_neg );

  compute_roe_flux( pL, pR,  normal, flux_pos , wave_speed );
  std::cout << flux_pos << std::endl;

  compute_roe_flux( pR, pL, -normal, flux_neg , wave_speed );
  std::cout << flux_neg << std::endl;

  BOOST_CHECK_EQUAL ( flux_pos, -flux_neg );

  compute_hlle_flux( pL, pR,  normal, flux_pos , wave_speed );
  std::cout << flux_pos << std::endl;

  compute_hlle_flux( pR, pL, -normal, flux_neg , wave_speed );
  std::cout << flux_neg << std::endl;

  BOOST_CHECK_EQUAL ( flux_pos, -flux_neg );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

