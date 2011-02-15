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
#define BOOST_TEST_MODULE "Test module for CF::Common 's parallel environment - part of testing collective communications."

////////////////////////////////////////////////////////////////////////////////

#include <vector>

#include <boost/test/unit_test.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/thread.hpp>

#include "Common/Log.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/MPI/tools.hpp"
#include "Common/MPI/datatype.hpp"
#include "Common/MPI/operations.hpp"
#include "Common/MPI/all_to_all.hpp"
#include "Common/MPI/all_reduce.hpp"
#include "Common/MPI/reduce.hpp"
#include "Common/MPI/scatter.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace CF;
using namespace CF::Common;

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
  mpi::PE::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL( mpi::PE::instance().is_init() , true );
  PEProcessSortedExecute(mpi::PE::instance(),-1,CFinfo << "Proccess " << mpi::PE::instance().rank() << "/" << mpi::PE::instance().size() << " reports in." << CFendl;);
}

////////////////////////////////////////////////////////////////////////////////

#include "test/Common/utest-parallel-collective-all_to_all.hpp"
#include "test/Common/utest-parallel-collective-all_reduce.hpp"
#include "test/Common/utest-parallel-collective-reduce.hpp"
#include "test/Common/utest-parallel-collective-scatter.hpp"

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( finalize, PECollectiveFixture )
{
  PEProcessSortedExecute(mpi::PE::instance(),-1,CFinfo << "Proccess " << mpi::PE::instance().rank() << "/" << mpi::PE::instance().size() << " says good bye." << CFendl;);
  mpi::PE::instance().finalize();
  BOOST_CHECK_EQUAL( mpi::PE::instance().is_init() , false );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

