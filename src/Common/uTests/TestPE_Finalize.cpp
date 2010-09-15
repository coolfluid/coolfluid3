// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

// IMPORTANT:
// run it both on 1 and 4 cores

#include "Common/MPI/PEInterface.hpp"
#include "Common/Log.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

struct PE_Finalize_Fixture
{
  /// common setup for each test case
  PE_Finalize_Fixture()
  {
  }

  /// common tear-down for each test case
  ~PE_Finalize_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here

};

BOOST_FIXTURE_TEST_SUITE( PE_Finalize_TestSuite, PE_Finalize_Fixture )

BOOST_AUTO_TEST_CASE( finalize )
{
  CFinfo << "TestPE_Finalize" << CFendl;

  PEInterface::instance().finalize();
  BOOST_CHECK_EQUAL( PEInterface::instance().is_init() , false );
}

BOOST_AUTO_TEST_SUITE_END()



