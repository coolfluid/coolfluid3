// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::physics::NavierStokes::Cons2D"

#include <boost/test/unit_test.hpp>

#include "cf3/common/Log.hpp"
#include "cf3/common/Core.hpp"
#include "cf3/common/Environment.hpp"

#include "cf3/physics/navierstokes/navierstokes2d/Functions.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::physics::navierstokes::navierstokes2d;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( NavierStokes_Cons2D_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_navierstokes2d )
{
    Data p;

    p.mu = 4.;
    p.kappa = 0.5;
    p.U[0] = 2.;
    p.U[1] = 2.;
    p.grad_u << 3., 2.;
    p.grad_v << 2., 3.;
    p.grad_T << 3., 3.;

    RowVector_NEQS flux;
    ColVector_NDIM normal;
    normal << 1., 1.;
    compute_diffusive_flux(p,normal,flux);
    RowVector_NEQS check; check << 0, 24, 24, 99;
    BOOST_CHECK(flux == check);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

