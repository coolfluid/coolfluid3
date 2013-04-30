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
#include <numeric>

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

//  double dd ( int idx, int rank, int nproc ) { return idx * ( rank + 1. ); }
  double dd ( int idx, int rank, int nproc ) { return  rank * 100 + idx; }

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

BOOST_FIXTURE_TEST_CASE( all_reduce, PECollectiveFixture )
{
  // double

  double sum = 0.;
  for(Uint i = 0; i < PE::Comm::instance().size();  ++i)
    sum += (double) i ;

  double rk_result;
  double rk = PE::Comm::instance().rank();
  int rk_size = 1;

  PE::Comm::instance().all_reduce( PE::plus(), &rk, rk_size, &rk_result );

  BOOST_CHECK_EQUAL( rk_result, sum );

  // for vectors

  std::vector<double> v (2);
  v[0] = PE::Comm::instance().rank();
  v[1] = PE::Comm::instance().size();

  /// @todo this should not compile ( but it does ) - Tamas to fix?
#if 0
  PE::Comm::instance().all_reduce( Comm::max(), &v, v.size(), &v );
#endif

  // this works
  PE::Comm::instance().all_reduce( PE::plus(), v, v );

  // this is equivalent
//  PE::Comm::instance().all_reduce( Comm::plus(), &v[0], v.size(), &v[0] );

  BOOST_CHECK_EQUAL( v[0], sum );
  BOOST_CHECK_EQUAL( v[1], PE::Comm::instance().size() * PE::Comm::instance().size() );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( all_to_all_ring_topology, PECollectiveFixture )
{
  int nproc = PE::Comm::instance().size();
  int irank = PE::Comm::instance().rank();

  // neighbour to right
  int rkright = (irank + 1) % nproc;
  // neighbour to left
  int rkleft = (nproc+irank-1) % nproc;

//  std::cout << "rk left  [" << irank << "]" << rkleft  << std::endl;
//  std::cout << "rk right [" << irank << "]" << rkright << std::endl;

  // data vector

  std::vector<double> v (100);
  for(Uint i = 0; i < v.size();  ++i)
    v[i] = dd(i,irank,nproc);

  // mapping vectors

  std::vector<int> send_map (10); // send last 10 entries of data vector
  for(Uint i = 0; i < send_map.size(); ++i)
    send_map[i] = 90 + i;

  std::vector<int> send_num (nproc, 0); // nb of entries to send to each rank (assumes send_map consecutive ordering for each rank )
  send_num[rkright] = 10;

  BOOST_CHECK_EQUAL( std::accumulate( send_num.begin(), send_num.end(), 0),  send_map.size() );

  std::vector<int> recv_map (10); // receive 1st 10 entries of data vector
  for(Uint i = 0; i < recv_map.size(); ++i)
    recv_map[i] = i;

  std::vector<int> recv_num (nproc, 0); // nb of entries to receive from each rank (assumes recv_map consecutive ordering for each rank )
  recv_num[rkleft] = 10;

  BOOST_CHECK_EQUAL( std::accumulate( recv_num.begin(), recv_num.end(), 0),  recv_map.size() );

  // communications

  PE::Comm::instance().all_to_all( v, send_num, send_map,  v, recv_num, recv_map);

//  PEDebugVector(v,v.size()); // this prints debug info on a vector !!!

  // check received data
  for(Uint i = 0; i < 10; ++i)
    BOOST_CHECK_EQUAL( v[i],  dd(i+90,rkleft,nproc) );

  // check  un-changed data and sent data
  for(Uint i = 10; i < v.size(); ++i)
    BOOST_CHECK_EQUAL( v[i],  dd(i,irank,nproc) );

}

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

