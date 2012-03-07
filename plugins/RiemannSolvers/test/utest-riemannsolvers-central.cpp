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
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"

#include "physics/PhysModel.hpp"
#include "physics/Variables.hpp"
#include "RiemannSolvers/RiemannSolver.hpp"

#include "math/Defs.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::RiemannSolvers;
using namespace cf3::physics;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( RiemannSolvers_Suite )

BOOST_AUTO_TEST_CASE( NavierStokes1D_Roe )
{
  Component& model =  *Core::instance().root().create_component<Component>("model1D");

  // Creation of physics + variables
  PhysModel& physics = *model.create_component("scalar1D","cf3.physics.Scalar.Scalar1D")->handle<PhysModel>();
  physics.options().configure_option("v",1.);
  Variables& sol_vars = *physics.create_variables("LinearAdv1D","solution");

  // Creation + configuration of riemann solver
  RiemannSolver& riemann = *model.create_component("riemann","cf3.RiemannSolvers.Central")->handle<RiemannSolver>();
  riemann.options().configure_option("physical_model",physics.handle<Component>());
  riemann.options().configure_option("solution_vars",sol_vars.handle<Component>());

  std::cout << model.tree() << std::endl;

  // Check simple flux computation
  Uint dim  = physics.ndim();
  Uint neqs = physics.neqs();
  RealVector normal(dim);
  RealVector left(neqs), right(neqs);
  RealVector flux(neqs);
  RealVector wave_speeds(neqs);

  normal << 1.;
  left << 2.;
  right << 4.;

  riemann.compute_interface_flux_and_wavespeeds(left,right, normal, normal, flux, wave_speeds);

  const Real tol (0.000001);
  BOOST_CHECK_CLOSE(flux[0] , 3. , tol);

  BOOST_CHECK_CLOSE(wave_speeds[0],  1. , tol);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

