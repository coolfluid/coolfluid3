// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Physics::NavierStokes::Cons2D"

#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/CEnv.hpp"

#include "NavierStokes/Cons2D.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Physics::NavierStokes;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( NavierStokes_Cons2D_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( compute_properties )
{
#if 0 // work in progress
  // create the model

  Physics::PhysModel& pmodel =
      Core::instance().root().create_component<NavierStokes2D>( "pmodel" );

  // values for testing

  NavierStokes2D::GeoV coords;

  coords[XX] = 0.0;
  coords[YY] = 2.0;
	
  NavierStokes2D::SolV vars;
	
	vars[Cons2D::Rho ] = 1.0;
	vars[Cons2D::RhoU] = 2.83972;
	vars[Cons2D::RhoV] = 0.0;
	vars[Cons2D::RhoE] = 6.532;

  NavierStokes2D::SolM grad_vars;
#endif

}

BOOST_AUTO_TEST_CASE( flux )
{
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

