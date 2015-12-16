// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
//
// IMPORTANT:
// run it both on 1 and many cores
// for example: mpirun -np 4 ./test-parallel-environment --report_level=confirm or --report_level=detailed

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::common 's parallel environment - part of testing the environment."

////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <boost/test/unit_test.hpp>

////////////////////////////////////////////////////////////////////////////////

#include "common/Log.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct PEFixture
{
  /// common setup for each test case
  PEFixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~PEFixture()
  {
  }

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;
  
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( PESuite, PEFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( isinit_preinit )
{
  BOOST_CHECK_EQUAL( PE::Comm::instance().is_active() , false );
}

BOOST_AUTO_TEST_CASE( allrankzero_preinit )
{
  BOOST_CHECK_EQUAL( PE::Comm::instance().rank() , (Uint)0 );
}

BOOST_AUTO_TEST_CASE( init )
{
  PE::Comm::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL( PE::Comm::instance().is_active() , true );
  PEProcessSortedExecute(-1,CFinfo << "Proccess " << PE::Comm::instance().rank() << "/" << PE::Comm::instance().size() << " reports in." << CFendl;);
}

BOOST_AUTO_TEST_CASE( rank_and_size )
{
  BOOST_CHECK_LT( PE::Comm::instance().rank() , PE::Comm::instance().size() );
}

BOOST_AUTO_TEST_CASE( finalize )
{
  PEProcessSortedExecute(-1,CFinfo << "Proccess " << PE::Comm::instance().rank() << "/" << PE::Comm::instance().size() << " says good bye." << CFendl;);
  PE::Comm::instance().finalize();
  BOOST_CHECK_EQUAL( PE::Comm::instance().is_active() , false );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

