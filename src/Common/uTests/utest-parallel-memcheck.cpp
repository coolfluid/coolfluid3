// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
//
// IMPORTANT:
// run it both on 1 and many cores
// for example: mpirun -np 4 ./test-parallel-environment --report_level=confirm or --report_level=detailed

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Common 's parallel environment - part of memcecking a parallel component creation."

////////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "Common/Log.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/MPI/PEObjectWrapper.hpp"
#include "Common/MPI/PECommPattern2.hpp"
#include "Common/CGroup.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct PEMemcheckFixture
{
  /// common setup for each test case
  PEMemcheckFixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~PEMemcheckFixture()
  {
    PE::instance().finalize();
  }

  /// common params
  int m_argc;
  char** m_argv;
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( PEMemcheckSuite, PEMemcheckFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  PE::instance().init(m_argc,m_argv);
  PE::instance().finalize();
//  BOOST_CHECK_EQUAL( PE::instance().is_init() , true );
}

////////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE( single_component_creation )
{
  //PECommPattern2 pecp("CommPattern2");
}
*/
////////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE( finalize )
{
  PE::instance().finalize();
  BOOST_CHECK_EQUAL( PE::instance().is_init() , false );
}
*/
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

