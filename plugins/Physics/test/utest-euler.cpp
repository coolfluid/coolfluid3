// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::Euler"

#include <boost/test/unit_test.hpp>


#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "Euler/Physics.hpp"
#include "Euler/Cons1D.hpp"
#include "Euler/Roe1D.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::Euler;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( Euler_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( eulercons1d )
{
  common::Core::instance().environment().options().set("log_level",(Uint)DEBUG);
  Euler::Cons1D cons_state;
  Euler::Roe1D  roe_state;
  boost::shared_ptr<solver::Physics> phys = cons_state.create_physics();
  //Euler::Physics p;
  solver::Physics& p = *phys;
  p.set_var(Euler::Physics::gamma,1.4);
  p.set_var(Euler::Physics::R,286.9);
  p.set_var(Euler::Physics::rho,4.696);
  p.set_var(Euler::Physics::Vx,0.);
  p.set_var(Euler::Physics::Vy,0.);
  p.set_var(Euler::Physics::Vz,0.);
  p.set_var(Euler::Physics::p,404400.);

  RealVector sol(3);
  cons_state.get_state(p,sol);

  RealVector roe(3);
  roe_state.get_state(p,roe);

  cons_state.set_state(sol,p);
  CFinfo << p << CFendl;
  roe_state.set_state(roe,p);
  CFinfo << p << CFendl;
  RealVector normal(1); normal << 1.;
  RealVector flux(3);
  RealMatrix rv(3,3);
  RealMatrix lv(3,3);
  RealVector ev(3);

  cons_state.compute_flux(p,normal,flux);
  CFLogVar(flux);
  //RealVector F_L(3); F_L << 0., 404400, 0.;

  cons_state.compute_fluxjacobian_eigenvalues(p,normal,ev);
  CFLogVar(ev);
  cons_state.compute_fluxjacobian_right_eigenvectors(p,normal,rv);
  CFLogVar(rv);
  cons_state.compute_fluxjacobian_left_eigenvectors(p,normal,lv);
  CFLogVar(lv);

//  BOOST_CHECK_EQUAL(flux[0], 5.);
//  BOOST_CHECK_EQUAL(ev[0], 1.);
//  BOOST_CHECK_EQUAL(rv(0,0), 1.);
//  BOOST_CHECK_EQUAL(lv(0,0), 1.);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

