// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::RiemannSolvers"

#include <boost/test/unit_test.hpp>

#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/OptionList.hpp"

#include "RiemannSolvers/RiemannSolver.hpp"
#include "Physics/NavierStokes/Cons1D.hpp"
#include "Physics/NavierStokes/Cons2D.hpp"
#include "Physics/NavierStokes/Cons3D.hpp"
#include "Physics/NavierStokes/Roe1D.hpp"
#include "Physics/NavierStokes/Roe2D.hpp"
#include "Physics/NavierStokes/Roe3D.hpp"
#include "math/Defs.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::RiemannSolvers;
using namespace cf3::physics;
using namespace cf3::physics::NavierStokes;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( RiemannSolvers_Suite )

BOOST_AUTO_TEST_CASE( NavierStokes1D_Roe )
{
  Component& model =  *Core::instance().root().create_component<Component>("model1D");

  // Creation of physics + variables
  Handle<PhysModel> physics( model.create_component("navierstokes","cf3.physics.NavierStokes.NavierStokes1D") );
  Handle<Variables> sol_vars( physics->create_variables("Cons1D","solution") );
  Handle<Variables> roe_vars( physics->create_variables("Roe1D","roe") );

  // Creation + configuration of riemann solver
  Handle<RiemannSolver> riemann( model.create_component("riemann","cf3.RiemannSolvers.Roe") );
  riemann->options().set("physical_model",physics);
  riemann->options().set("solution_vars",sol_vars);
  riemann->options().set("roe_vars",roe_vars);

  std::cout << model.tree() << std::endl;

  // Check simple flux computation
  Uint dim  = physics->ndim();
  Uint neqs = physics->neqs();
  RealVector normal(dim);
  RealVector left(neqs), right(neqs);
  RealVector flux(neqs);
  RealVector wave_speeds(neqs);

  const Real r_L = 4.696;     const Real r_R = 1.408;
  const Real u_L = 0.;        const Real u_R = 0.;
  const Real p_L = 404400;    const Real p_R = 101100;
  const Real g = 1.4;

  normal << 1.;
  left <<  r_L, r_L*u_L, p_L/(g-1.) + 0.5*r_L*(u_L*u_L);
  right << r_R, r_R*u_R, p_R/(g-1.) + 0.5*r_R*(u_R*u_R);

  RealVector coords(dim); coords.setZero();

  riemann->compute_interface_flux_and_wavespeeds(left,right, coords,normal, flux, wave_speeds);

  const Real tol (0.000001);
  BOOST_CHECK_CLOSE(flux[0] , 450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] , 252750 , tol);
  BOOST_CHECK_CLOSE(flux[2] , 127710965.918678 , tol);

  BOOST_CHECK_CLOSE(wave_speeds[0],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[1],  336.8571471643333 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[2], -336.8571471643333 , tol);

  riemann->compute_interface_flux_and_wavespeeds(right,left,coords, -normal, flux, wave_speeds);

  BOOST_CHECK_CLOSE(flux[0] , -450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] , -252750 , tol);
  BOOST_CHECK_CLOSE(flux[2] , -127710965.918678 , tol);

  BOOST_CHECK_CLOSE(wave_speeds[0],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[1],  336.8571471643333 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[2], -336.8571471643333 , tol);
}


BOOST_AUTO_TEST_CASE( NavierStokes2D_Roe )
{
  Component& model =  *Core::instance().root().create_component<Component>("model2D");

  // Creation of physics + variables
  Handle<PhysModel> physics( model.create_component("navierstokes","cf3.physics.NavierStokes.NavierStokes2D") );
  Handle<Variables> sol_vars( physics->create_variables("Cons2D","solution") );
  Handle<Variables> roe_vars( physics->create_variables("Roe2D","roe") );

  // Creation + configuration of riemann solver
  Handle<RiemannSolver> riemann( model.create_component("riemann","cf3.RiemannSolvers.Roe") );
  riemann->options().set("physical_model",physics);
  riemann->options().set("solution_vars",sol_vars);
  riemann->options().set("roe_vars",roe_vars);

  std::cout << model.tree() << std::endl;


  // Check simple flux computation
  Uint dim  = physics->ndim();
  Uint neqs = physics->neqs();
  RealVector normal(dim);
  RealVector left(neqs), right(neqs);
  RealVector flux(neqs);
  RealVector wave_speeds(neqs);
  RealVector coords(dim); coords.setZero();

  const Real r_L = 4.696;     const Real r_R = 1.408;
  const Real u_L = 0.;        const Real u_R = 0.;
  const Real v_L = 0.;        const Real v_R = 0.;
  const Real p_L = 404400;    const Real p_R = 101100;
  const Real g = 1.4;

  normal << 1. , 0.;
  left <<  r_L, r_L*u_L, r_L*v_L, p_L/(g-1.) + 0.5*r_L*(u_L*u_L+v_L*v_L);
  right << r_R, r_R*u_R, r_R*v_R, p_R/(g-1.) + 0.5*r_R*(u_R*u_R+v_R*v_R);

  riemann->compute_interface_flux_and_wavespeeds(left,right, coords, normal, flux, wave_speeds);
  std::cout << "compute_riemann_problem( " << left.transpose() << "   ,   " << right.transpose() << "    ,    " << normal.transpose() << "  ) " << std::endl;
  std::cout << "  =   " << flux << std::endl;

  const Real tol (0.000001);
  BOOST_CHECK_CLOSE(flux[0] , 450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] , 252750 , tol);
  BOOST_CHECK_CLOSE(flux[2] , 0      , tol);
  BOOST_CHECK_CLOSE(flux[3] , 127710965.918678 , tol);

  BOOST_CHECK_CLOSE(wave_speeds[0],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[1],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[2],  336.8571471643333 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[3], -336.8571471643333 , tol);

  riemann->compute_interface_flux_and_wavespeeds(right,left, coords, -normal, flux, wave_speeds);

  BOOST_CHECK_CLOSE(flux[0] , -450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] , -252750 , tol);
  BOOST_CHECK_CLOSE(flux[2] ,  0 , tol);
  BOOST_CHECK_CLOSE(flux[3] , -127710965.918678 , tol);

  BOOST_CHECK_CLOSE(wave_speeds[0],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[1],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[2],  336.8571471643333 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[3], -336.8571471643333 , tol);

  normal << 0., 1.;
  riemann->compute_interface_flux_and_wavespeeds(left,right,coords,normal,flux,wave_speeds);
  BOOST_CHECK_CLOSE(flux[0] , 450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] , 0       , tol);
  BOOST_CHECK_CLOSE(flux[2] , 252750  , tol);
  BOOST_CHECK_CLOSE(flux[3] , 127710965.918678 , tol);

  BOOST_CHECK_CLOSE(wave_speeds[0],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[1],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[2],  336.8571471643333 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[3], -336.8571471643333 , tol);
  riemann->compute_interface_flux_and_wavespeeds(right,left,coords,-normal,flux,wave_speeds);
  BOOST_CHECK_CLOSE(flux[0] , -450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] ,  0       , tol);
  BOOST_CHECK_CLOSE(flux[2] , -252750  , tol);
  BOOST_CHECK_CLOSE(flux[3] , -127710965.918678 , tol);


  normal << 1., 1.;
  riemann->compute_interface_flux_and_wavespeeds(left,right,coords,normal,flux,wave_speeds);
  BOOST_CHECK_CLOSE(flux[1] , flux[2] , tol);
  riemann->compute_interface_flux_and_wavespeeds(right,left,coords,-normal,flux,wave_speeds);
  BOOST_CHECK_CLOSE(flux[1] , flux[2] , tol);
  BOOST_CHECK_CLOSE(wave_speeds[0],  0 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[1],  0 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[2],  336.8571471643333 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[3], -336.8571471643333 , tol);


}

BOOST_AUTO_TEST_CASE( NavierStokes3D_Roe )
{
  Component& model =  *Core::instance().root().create_component<Component>("model3D");

  // Creation of physics + variables
  Handle<PhysModel> physics( model.create_component("navierstokes","cf3.physics.NavierStokes.NavierStokes3D") );
  Handle<Variables> sol_vars( physics->create_variables("Cons3D","solution") );
  Handle<Variables> roe_vars( physics->create_variables("Roe3D","roe") );

  // Creation + configuration of riemann solver
  Handle<RiemannSolver> riemann( model.create_component("riemann","cf3.RiemannSolvers.Roe") );
  riemann->options().set("physical_model",physics);
  riemann->options().set("solution_vars",sol_vars);
  riemann->options().set("roe_vars",roe_vars);

  std::cout << model.tree() << std::endl;


  // Check simple flux computation
  Uint dim  = physics->ndim();
  Uint neqs = physics->neqs();
  RealVector normal(dim);
  RealVector left(neqs), right(neqs);
  RealVector flux(neqs);
  RealVector wave_speeds(neqs);
  const Real tol (0.000001);
  RealVector coords(dim); coords.setZero();


  const Real r_L = 4.696;     const Real r_R = 1.408;
  const Real u_L = 0.;        const Real u_R = 0.;
  const Real v_L = 0.;        const Real v_R = 0.;
  const Real w_L = 0.;        const Real w_R = 0.;
  const Real p_L = 404400;    const Real p_R = 101100;
  const Real g = 1.4;

  normal << 1. , 0. , 0.;
  left <<  r_L, r_L*u_L, r_L*v_L, r_L*w_L, p_L/(g-1.) + 0.5*r_L*(u_L*u_L+v_L*v_L+w_L*w_L);
  right << r_R, r_R*u_R, r_R*v_R, r_R*w_R, p_R/(g-1.) + 0.5*r_R*(u_R*u_R+v_R*v_R+w_R*w_R);

  riemann->compute_interface_flux_and_wavespeeds(left,right, coords, normal, flux, wave_speeds);

  BOOST_CHECK_CLOSE(flux[0] , 450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] , 252750 , tol);
  BOOST_CHECK_CLOSE(flux[2] , 0      , tol);
  BOOST_CHECK_CLOSE(flux[3] , 0      , tol);
  BOOST_CHECK_CLOSE(flux[4] , 127710965.918678 , tol);

  BOOST_CHECK_CLOSE(wave_speeds[0],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[1],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[2],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[3],  336.8571471643333 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[4], -336.8571471643333 , tol);

  riemann->compute_interface_flux_and_wavespeeds(right, left, coords, -normal, flux, wave_speeds);

  BOOST_CHECK_CLOSE(flux[0] , -450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] , -252750 , tol);
  BOOST_CHECK_CLOSE(flux[2] ,  0      , tol);
  BOOST_CHECK_CLOSE(flux[3] ,  0      , tol);
  BOOST_CHECK_CLOSE(flux[4] , -127710965.918678 , tol);

  BOOST_CHECK_CLOSE(wave_speeds[0],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[1],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[2],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[3],  336.8571471643333 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[4], -336.8571471643333 , tol);

  normal << 0. , 1. , 0.;

  riemann->compute_interface_flux_and_wavespeeds(left,right, coords, normal, flux, wave_speeds);

  BOOST_CHECK_CLOSE(flux[0] , 450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] , 0      , tol);
  BOOST_CHECK_CLOSE(flux[2] , 252750 , tol);
  BOOST_CHECK_CLOSE(flux[3] , 0      , tol);
  BOOST_CHECK_CLOSE(flux[4] , 127710965.918678 , tol);

  BOOST_CHECK_CLOSE(wave_speeds[0],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[1],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[2],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[3],  336.8571471643333 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[4], -336.8571471643333 , tol);

  riemann->compute_interface_flux_and_wavespeeds(right, left, coords, -normal, flux, wave_speeds);

  BOOST_CHECK_CLOSE(flux[0] , -450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] ,  0      , tol);
  BOOST_CHECK_CLOSE(flux[2] , -252750 , tol);
  BOOST_CHECK_CLOSE(flux[3] ,  0      , tol);
  BOOST_CHECK_CLOSE(flux[4] , -127710965.918678 , tol);

  BOOST_CHECK_CLOSE(wave_speeds[0],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[1],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[2],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[3],  336.8571471643333 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[4], -336.8571471643333 , tol);

  normal << 0. , 0. , 1.;

  riemann->compute_interface_flux_and_wavespeeds(left,right, coords, normal, flux, wave_speeds);

  BOOST_CHECK_CLOSE(flux[0] , 450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] , 0      , tol);
  BOOST_CHECK_CLOSE(flux[2] , 0      , tol);
  BOOST_CHECK_CLOSE(flux[3] , 252750 , tol);
  BOOST_CHECK_CLOSE(flux[4] , 127710965.918678 , tol);

  BOOST_CHECK_CLOSE(wave_speeds[0],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[1],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[2],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[3],  336.8571471643333 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[4], -336.8571471643333 , tol);

  riemann->compute_interface_flux_and_wavespeeds(right, left, coords, -normal, flux, wave_speeds);

  BOOST_CHECK_CLOSE(flux[0] , -450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] ,  0      , tol);
  BOOST_CHECK_CLOSE(flux[2] ,  0      , tol);
  BOOST_CHECK_CLOSE(flux[3] , -252750 , tol);
  BOOST_CHECK_CLOSE(flux[4] , -127710965.918678 , tol);

  BOOST_CHECK_CLOSE(wave_speeds[0],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[1],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[2],  0                 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[3],  336.8571471643333 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[4], -336.8571471643333 , tol);


  normal << 1., 1., 1.;
  riemann->compute_interface_flux_and_wavespeeds(left,right,coords,normal,flux,wave_speeds);
  BOOST_CHECK_CLOSE(flux[1] , flux[2] , tol);
  BOOST_CHECK_CLOSE(flux[2] , flux[3] , tol);
  riemann->compute_interface_flux_and_wavespeeds(right,left,coords,-normal,flux,wave_speeds);
  BOOST_CHECK_CLOSE(flux[1] , flux[2] , tol);
  BOOST_CHECK_CLOSE(flux[2] , flux[3] , tol);
  BOOST_CHECK_CLOSE(wave_speeds[0],  0 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[1],  0 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[2],  0 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[3],  336.8571471643333 , tol);
  BOOST_CHECK_CLOSE(wave_speeds[4], -336.8571471643333 , tol);

}
#if 0
//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_Roe_adv_diff )
{
  RiemannSolver& riemannsolver = Core::instance().root().create_component("Roe-solver-AdvectionDiffusion1D","cf3.RiemannSolvers.Roe").as_type<RiemannSolver>();

  Component& state = Core::instance().root().create_component("solution-state-AdvectionDiffusion1D","cf3.AdvectionDiffusion.State1D");
  riemannsolver.options().set("solution_state",state.uri());
  riemannsolver.options().set("roe_state",std::string("cf3.AdvectionDiffusion.State1D"));

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

#endif
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

