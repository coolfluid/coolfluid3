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
#include <numeric>

#include <boost/test/unit_test.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/thread.hpp>

#include "Common/Log.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/MPI/debug.hpp"
#include "Common/MPI/datatype.hpp"
#include "Common/MPI/operations.hpp"
#include "Common/MPI/all_to_all.hpp"
#include "Common/MPI/all_reduce.hpp"
#include "Common/MPI/reduce.hpp"
#include "Common/MPI/scatter.hpp"
#include "Common/MPI/broadcast.hpp"
#include "Common/MPI/gather.hpp"
#include "Common/MPI/all_gather.hpp"

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
  mpi::PE::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL( mpi::PE::instance().is_active() , true );
  CFinfo.setFilterRankZero(false);
  PEProcessSortedExecute(-1,CFinfo << "Proccess " << mpi::PE::instance().rank() << "/" << mpi::PE::instance().size() << " reports in." << CFendl;);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( all_reduce, PECollectiveFixture )
{
  // double

  double sum = 0.;
  for(Uint i = 0; i < mpi::PE::instance().size();  ++i)
    sum += (double) i ;

  double rk_result;
  double rk = mpi::PE::instance().rank();
  int rk_size = 1;

  mpi::PE::instance().all_reduce( mpi::plus(), &rk, rk_size, &rk_result );

  BOOST_CHECK_EQUAL( rk_result, sum );

  // for vectors

  std::vector<double> v (2);
  v[0] = mpi::PE::instance().rank();
  v[1] = mpi::PE::instance().size();

  /// @todo this should not compile ( but it does ) - Tamas to fix?
#if 0
  mpi::PE::instance().all_reduce( mpi::max(), &v, v.size(), &v );
#endif

  // this works
  mpi::PE::instance().all_reduce( mpi::plus(), v, v );

  // this is equivalent
//  mpi::PE::instance().all_reduce( mpi::plus(), &v[0], v.size(), &v[0] );

  BOOST_CHECK_EQUAL( v[0], sum );
  BOOST_CHECK_EQUAL( v[1], mpi::PE::instance().size() * mpi::PE::instance().size() );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( all_to_all_ring_topology, PECollectiveFixture )
{
  int nproc = mpi::PE::instance().size();
  int irank = mpi::PE::instance().rank();

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

  mpi::PE::instance().all_to_all( v, send_num, send_map,  v, recv_num, recv_map);

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
  PEProcessSortedExecute(-1,CFinfo << "Proccess " << mpi::PE::instance().rank() << "/" << mpi::PE::instance().size() << " says good bye." << CFendl;);
  CFinfo.setFilterRankZero(true);
  mpi::PE::instance().finalize();
  BOOST_CHECK_EQUAL( mpi::PE::instance().is_active() , false );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

