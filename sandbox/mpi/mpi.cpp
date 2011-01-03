// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for MPI"

#include <mpi.h>
#include <iostream>

#include <boost/test/unit_test.hpp>

#include "Common/CF.hpp"

using namespace std;

//////////////////////////////////////////////////////////////////////////////

using namespace CF;
using namespace CF::Common;

struct MyGlobalFixture
{
  /// common setup for each test case
  MyGlobalFixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;

    cout << "mpi init" << endl;
    MPI_Init(&m_argc,&m_argv);
  }

  /// common tear-down for each test case
  ~MyGlobalFixture()
  {
    cout << "mpi finalize" << endl;
    MPI_Finalize();
  }

  /// common data
  int    m_argc;
  char** m_argv;
};

struct MyFixture
{
};

//////////////////////////////////////////////////////////////////////////////

BOOST_GLOBAL_FIXTURE( MyGlobalFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( MySuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( test_echo1, MyFixture )
{
  int myid, numprocs;

  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);

  cout << "Hello from proc " << myid << " of " << numprocs << endl;
}

//////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( test_echo2, MyFixture )
{
  int myid, numprocs;

  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);

  cout << "Hello from proc " << myid << " of " << numprocs << endl;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
