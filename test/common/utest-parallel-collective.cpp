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
#define BOOST_TEST_MODULE "Test module for cf3::common 's parallel environment - part of testing collective communications."

////////////////////////////////////////////////////////////////////////////////

#include <vector>

#include <boost/test/unit_test.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/thread.hpp>

#include "common/Log.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"
#include "common/PE/datatype.hpp"
#include "common/PE/operations.hpp"
#include "common/PE/all_to_all.hpp"
#include "common/PE/all_reduce.hpp"
#include "common/PE/reduce.hpp"
#include "common/PE/scatter.hpp"
#include "common/PE/broadcast.hpp"
#include "common/PE/gather.hpp"
#include "common/PE/all_gather.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct PECollectiveFixture
{
  /// common setup for each test case
  PECollectiveFixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~PECollectiveFixture() { }

  /// common data
  int m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( PECollectiveSuite, PECollectiveFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( init, PECollectiveFixture )
{
  PE::Comm::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL( PE::Comm::instance().is_active() , true );
  CFinfo.setFilterRankZero(false);
  PEProcessSortedExecute(-1,CFinfo << "Proccess " << PE::Comm::instance().rank() << "/" << PE::Comm::instance().size() << " reports in." << CFendl;);
}

////////////////////////////////////////////////////////////////////////////////

#include "test/common/utest-parallel-collective-all_to_all.hpp"
#include "test/common/utest-parallel-collective-all_reduce.hpp"
#include "test/common/utest-parallel-collective-reduce.hpp"
#include "test/common/utest-parallel-collective-scatter.hpp"
#include "test/common/utest-parallel-collective-broadcast.hpp"
#include "test/common/utest-parallel-collective-all_gather.hpp"
#include "test/common/utest-parallel-collective-gather.hpp"

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( finalize, PECollectiveFixture )
{
  PEProcessSortedExecute(-1,CFinfo << "Proccess " << PE::Comm::instance().rank() << "/" << PE::Comm::instance().size() << " says good bye." << CFendl;);
  CFinfo.setFilterRankZero(true);
  PE::Comm::instance().finalize();
  BOOST_CHECK_EQUAL( PE::Comm::instance().is_active() , false );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

