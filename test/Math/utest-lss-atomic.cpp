// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
//

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Math::LSS where testing indivdual operations."

////////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Math/LSS/System.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace CF;
using namespace CF::Math;
using namespace CF::Math::LSS;

////////////////////////////////////////////////////////////////////////////////

struct LSSAtomicFixture
{
  /// common setup for each test case
  LSSAtomicFixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~LSSAtomicFixture()
  {
  }

  /// common params
  int m_argc;
  char** m_argv;
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( LSSAtomicSuite, LSSAtomicFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( hello world )
{
  CFinfo << "Hello World!\n";
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

