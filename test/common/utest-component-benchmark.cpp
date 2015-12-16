// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::Component"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/Component.hpp"
#include "common/Group.hpp"

#include "Tools/Testing/ProfiledTestFixture.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"

using namespace cf3;

struct ComponentBenchFixture : Tools::Testing::TimedTestFixture, Tools::Testing::ProfiledTestFixture
{
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( ComponentBenchSuite, ComponentBenchFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructor )
{
  for(Uint i = 0; i != 10000; ++i)
    common::allocate_component<common::Group>("test");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
