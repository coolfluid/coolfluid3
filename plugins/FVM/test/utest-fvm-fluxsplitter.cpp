// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::FVM"

#include <boost/test/unit_test.hpp>


#include "Common/Log.hpp"

#include "Mesh/LibMesh.hpp"

#include "FVM/Core/RoeCons1D.hpp"
#include "FVM/Core/RoeCons2D.hpp"

using namespace CF;
using namespace CF::FVM;
using namespace CF::FVM::Core;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( FVM_FluxSplitter_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Roe1d )
{
  RoeCons1D roe("roe");

  RealVector left(3);
  RealVector right(3);
  RealVector normal(1);
  normal[XX] = 1.;

  Real g=1.4;

  const Real r_L = 4.696;     const Real r_R = 1.408;
  const Real u_L = 0.;        const Real u_R = 0.;
  const Real p_L = 404400;    const Real p_R = 101100;

  left <<  r_L, r_L*u_L, p_L/(g-1.) + 0.5*r_L*u_L*u_L;
  right << r_R, r_R*u_R, p_R/(g-1.) + 0.5*r_R*u_R*u_R;

  RealVector F_L(3); F_L << 0., 404400, 0.;
  RealVector F_R(3); F_R << 0., 101100, 0.;
  BOOST_CHECK_EQUAL(roe.flux(left,normal),F_L);
  BOOST_CHECK_EQUAL(roe.flux(right,normal),F_R);
  const Real tol (0.000001);
  BOOST_CHECK_CLOSE(roe.interface_flux(left,right,normal)[0] , 450.190834 , tol);
  BOOST_CHECK_CLOSE(roe.interface_flux(left,right,normal)[1] , 252750 , tol);
  BOOST_CHECK_CLOSE(roe.interface_flux(left,right,normal)[2] , 127710965.918678 , tol);

  BOOST_CHECK_CLOSE(roe.interface_flux(right,left,-normal)[0] , -450.190834 , tol);
  BOOST_CHECK_CLOSE(roe.interface_flux(right,left,-normal)[1] , -252750 , tol);
  BOOST_CHECK_CLOSE(roe.interface_flux(right,left,-normal)[2] , -127710965.918678 , tol);


  RealVector roe_avg(3);
  roe.compute_roe_average(left,right,roe_avg);

  BOOST_CHECK_CLOSE(roe_avg[0] , 2.5713747295950466 , tol);
  BOOST_CHECK_CLOSE(roe_avg[1] , 0. , tol);
  BOOST_CHECK_CLOSE(roe_avg[2] , 521037.37491345644 , tol);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Roe2d )
{
  RoeCons2D roe("roe");

  RealVector left(4);
  RealVector right(4);
  RealVector normal(2);
  RealVector F_L(4);
  RealVector F_R(4);
  const Real tol (0.000001);

  Real g=1.4;

  const Real r_L = 4.696;     const Real r_R = 1.408;
  const Real u_L = 0.;        const Real u_R = 0.;
  const Real v_L = 0.;        const Real v_R = 0.;
  const Real p_L = 404400;    const Real p_R = 101100;

  left <<  r_L, r_L*u_L, r_L*v_L, p_L/(g-1.) + 0.5*r_L*(u_L*u_L+v_L*v_L);
  right << r_R, r_R*u_R, r_R*v_R, p_R/(g-1.) + 0.5*r_R*(u_R*u_R+v_R*v_R);


  F_L << 0., 404400, 0., 0.;
  F_R << 0., 101100, 0., 0.;
  normal << 1. , 0. ;
  BOOST_CHECK_EQUAL(roe.flux(left,normal),F_L);
  BOOST_CHECK_EQUAL(roe.flux(right,normal),F_R);
  BOOST_CHECK_CLOSE(roe.interface_flux(left,right,normal)[0] , 450.190834 , tol);
  BOOST_CHECK_CLOSE(roe.interface_flux(left,right,normal)[1] , 252750. , tol);
  BOOST_CHECK_CLOSE(roe.interface_flux(left,right,normal)[2] , 0. , tol);
  BOOST_CHECK_CLOSE(roe.interface_flux(left,right,normal)[3] , 127710965.918678 , tol);


  F_L << 0., 0., 404400, 0.;
  F_R << 0., 0., 101100, 0.;
  normal << 0. , 1. ;
  BOOST_CHECK_EQUAL(roe.flux(left,normal),F_L);
  BOOST_CHECK_EQUAL(roe.flux(right,normal),F_R);
  BOOST_CHECK_CLOSE(roe.interface_flux(left,right,normal)[0] , 450.190834 , tol);
  BOOST_CHECK_CLOSE(roe.interface_flux(left,right,normal)[1] , 0. , tol);
  BOOST_CHECK_CLOSE(roe.interface_flux(left,right,normal)[2] , 252750. , tol);
  BOOST_CHECK_CLOSE(roe.interface_flux(left,right,normal)[3] , 127710965.918678 , tol);

  normal << 1. , 1. ;
  normal.normalize();
  BOOST_CHECK_CLOSE(roe.interface_flux(left,right,normal)[1] , roe.interface_flux(left,right,normal)[2] , tol);

}
//
// BOOST_AUTO_TEST_CASE( compare_1d_2d )
// {
//   RoeFluxSplitterCons1D roe1d("roe1d");
//   RoeFluxSplitterCons2D roe2d("roe2d");
//
//
//   RealVector left1d(3);
//   RealVector right1d(3);
//   RealVector normal1d(1);
//   normal1d[XX] = 1.;
//
//   RealVector left2d(4);
//   RealVector right2d(4);
//   RealVector normal2d(2);
//   normal2d << 1. , 0. ;
//
//   Real g=1.4;
//
//   const Real r_L = 4.696;     const Real r_R = 1.408;
//   const Real u_L = 0.;        const Real u_R = 0.;
//   const Real v_L = 0.;        const Real v_R = 0.;
//   const Real p_L = 404400;    const Real p_R = 101100;
//
//   left1d <<  r_L, r_L*u_L, p_L/(g-1.) + 0.5*r_L*u_L*u_L;
//   right1d << r_R, r_R*u_R, p_R/(g-1.) + 0.5*r_R*u_R*u_R;
//
//   left2d <<  r_L, r_L*u_L, r_L*v_L, p_L/(g-1.) + 0.5*r_L*(u_L*u_L+v_L*v_L);
//   right2d << r_R, r_R*u_R, r_R*v_R, p_R/(g-1.) + 0.5*r_R*(u_R*u_R+v_R*v_R);
//
//   Real left_wave_speed1d, right_wave_speed1d;
//   RealVector flux1d(3);
//   roe1d.solve(left1d,right1d,normal1d, flux1d,left_wave_speed1d, right_wave_speed1d);
//
//   Real left_wave_speed2d, right_wave_speed2d;
//   RealVector flux2d(4);
//   roe2d.solve(left2d,right2d,normal2d, flux2d,left_wave_speed2d, right_wave_speed2d);
//
//   const Real tol (0.000001);
//
//   BOOST_CHECK_CLOSE(left_wave_speed1d  , left_wave_speed2d  , tol);
//   BOOST_CHECK_CLOSE(right_wave_speed1d , right_wave_speed2d , tol);
//
//
//   normal2d << 0. , 1. ;
//   roe2d.solve(left2d,left2d,normal2d, flux2d,left_wave_speed2d, right_wave_speed2d);
//   CFLogVar(left_wave_speed2d);
//   CFLogVar(right_wave_speed2d);
//
// }
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

