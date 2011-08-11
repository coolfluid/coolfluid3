// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::RiemannSolvers"

#include <boost/test/unit_test.hpp>


#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"

#include "Solver/State.hpp"
#include "RiemannSolvers/RiemannSolver.hpp"
#include "Math/Defs.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::RiemannSolvers;
using namespace CF::Solver;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( RiemannSolvers_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_Roe_adv_diff )
{
  RiemannSolver& riemannsolver = Core::instance().root().create_component("Roe-solver-AdvectionDiffusion1D","CF.RiemannSolvers.Roe").as_type<RiemannSolver>();

  Component& state = Core::instance().root().create_component("solution-state-AdvectionDiffusion1D","CF.AdvectionDiffusion.State1D");
  riemannsolver.configure_option("solution_state",state.uri());
  riemannsolver.configure_option("roe_state",std::string("CF.AdvectionDiffusion.State1D"));

  RealVector left(1);   left   << 1.5;
  RealVector right(1);  right  << 0.5;
  RealVector normal(1); normal << 1.;
  RealVector flux(1);
  Real left_wave_speed;
  Real right_wave_speed;
  riemannsolver.solve(left,right,normal,
                      flux,left_wave_speed,right_wave_speed);

  BOOST_CHECK_EQUAL( flux[0] , 1.5 );
  BOOST_CHECK_EQUAL( left_wave_speed  ,  1. );
  BOOST_CHECK_EQUAL( right_wave_speed , -1. );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_Roe_euler1d )
{
  RiemannSolver& riemannsolver = Core::instance().root().create_component("Roe-solver-Euler1D","CF.RiemannSolvers.Roe").as_type<RiemannSolver>();

  Solver::State& state = Core::instance().root().create_component("solution-state-Euler1D","CF.Euler.Cons1D").as_type<Solver::State>();
  riemannsolver.configure_option("solution_state",state.uri());
  riemannsolver.configure_option("roe_state",std::string("CF.Euler.Roe1D"));

  RealVector left(3);
  RealVector right(3);

  Real g=1.4;

  const Real r_L = 4.696;     const Real r_R = 1.408;
  const Real u_L = 0.;        const Real u_R = 0.;
  const Real p_L = 404400;    const Real p_R = 101100;

  left <<  r_L, r_L*u_L, p_L/(g-1.) + 0.5*r_L*u_L*u_L;
  right << r_R, r_R*u_R, p_R/(g-1.) + 0.5*r_R*u_R*u_R;

  RealVector normal(1); normal << 1.;
  RealVector flux(3);
  Real left_wave_speed;
  Real right_wave_speed;
  riemannsolver.solve(left,right,normal,
                      flux,left_wave_speed,right_wave_speed);

  const Real tol (0.000001);
  BOOST_CHECK_CLOSE(flux[0] , 450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] , 252750 , tol);
  BOOST_CHECK_CLOSE(flux[2] , 127710965.918678 , tol);

  riemannsolver.solve(right,left,-normal,
                      flux,right_wave_speed,left_wave_speed);

  BOOST_CHECK_CLOSE(flux[0] , -450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] , -252750 , tol);
  BOOST_CHECK_CLOSE(flux[2] , -127710965.918678 , tol);

}

BOOST_AUTO_TEST_CASE( test_Roe_euler2d )
{
  RiemannSolver& riemannsolver = Core::instance().root().create_component("Roe-solver-Euler2D","CF.RiemannSolvers.Roe").as_type<RiemannSolver>();

  Solver::State& state = Core::instance().root().create_component("solution-state-Euler2D","CF.Euler.Cons2D").as_type<Solver::State>();
  riemannsolver.configure_option("solution_state",state.uri());
  riemannsolver.configure_option("roe_state",std::string("CF.Euler.Roe2D"));

  RealVector left(4);
  RealVector right(4);
  RealVector flux(4);
  Real left_wave_speed;
  Real right_wave_speed;
  RealVector normal(2);
  const Real tol (0.000001);

  Real g=1.4;

  const Real r_L = 4.696;     const Real r_R = 1.408;
  const Real u_L = 0.;        const Real u_R = 0.;
  const Real v_L = 0.;        const Real v_R = 0.;
  const Real p_L = 404400;    const Real p_R = 101100;

  left <<  r_L, r_L*u_L, r_L*v_L, p_L/(g-1.) + 0.5*r_L*(u_L*u_L+v_L*v_L);
  right << r_R, r_R*u_R, r_R*v_R, p_R/(g-1.) + 0.5*r_R*(u_R*u_R+v_R*v_R);


  normal << 1. , 0.;
  riemannsolver.solve(left,right,normal,
                      flux,left_wave_speed,right_wave_speed);
  BOOST_CHECK_CLOSE(flux[0] , 450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] , 252750 , tol);
  BOOST_CHECK_CLOSE(flux[2]+1. , 1. , tol);
  BOOST_CHECK_CLOSE(flux[3] , 127710965.918678 , tol);
  BOOST_CHECK_CLOSE(left_wave_speed, 336.8571471643333 , tol);

  riemannsolver.solve(right,left,-normal,
                      flux,right_wave_speed,left_wave_speed);
  BOOST_CHECK_CLOSE(flux[0] , -450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] , -252750 , tol);
  BOOST_CHECK_CLOSE(flux[2]+1. , 1. , tol);
  BOOST_CHECK_CLOSE(flux[3] , -127710965.918678 , tol);
  BOOST_CHECK_CLOSE(left_wave_speed, 336.8571471643333 , tol);

  normal << 0., 1.;
  riemannsolver.solve(left,right,normal,
                      flux,left_wave_speed,right_wave_speed);
  BOOST_CHECK_CLOSE(flux[0] , 450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1]+1. , 1. , tol);
  BOOST_CHECK_CLOSE(flux[2] , 252750 , tol);
  BOOST_CHECK_CLOSE(flux[3] , 127710965.918678 , tol);
  BOOST_CHECK_CLOSE(left_wave_speed, 336.8571471643333 , tol);

  riemannsolver.solve(right,left,-normal,
                      flux,right_wave_speed,left_wave_speed);
  BOOST_CHECK_CLOSE(flux[0] , -450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1]+1. , 1. , tol);
  BOOST_CHECK_CLOSE(flux[2] , -252750 , tol);
  BOOST_CHECK_CLOSE(flux[3] , -127710965.918678 , tol);
  BOOST_CHECK_CLOSE(left_wave_speed, 336.8571471643333 , tol);

  normal << 1., 1.;
  riemannsolver.solve(left,right,normal,
                      flux,left_wave_speed,right_wave_speed);
  BOOST_CHECK_CLOSE(flux[1] , flux[2] , tol);

  riemannsolver.solve(left,right,-normal,
                      flux,left_wave_speed,right_wave_speed);
  BOOST_CHECK_CLOSE(flux[1] , flux[2] , tol);
  BOOST_CHECK_CLOSE(flux[1] , flux[2] , tol);
  BOOST_CHECK_CLOSE(left_wave_speed, 336.8571471643333 , tol);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

