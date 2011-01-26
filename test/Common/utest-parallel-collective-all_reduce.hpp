// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// this file is en-block included into utest-parallel-collective.cpp
// do not include anything here, rather in utest-parallel-collective.cpp

////////////////////////////////////////////////////////////////////////////////

struct PEAllReduceFixture
{
  /// common setup for each test case
  PEAllReduceFixture()
  {
    // rank and proc
    nproc=mpi::PE::instance().size();
    irank=mpi::PE::instance().rank();
  }

  /// common tear-down for each test case
  ~PEAllReduceFixture()
  {
  }

  /// number of processes
  int nproc;
  /// rank of process
  int irank;

  /// helper function for constant size data - setting up input and verification data
  void setup_data_constant()
  {
  }

  /// helper function for variable size data - setting up input and verification data
  void setup_data_variable()
  {
  }

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( PEAllReduceSuite, PEAllReduceFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( all_reduce_pre_test )
{
  int ivec[3];
  ivec[0]=0*mpi::PE::instance().size();
  ivec[1]=1*mpi::PE::instance().size();
  ivec[2]=2*mpi::PE::instance().size();
  int ovec[3];
  //mpi::all_reduce(mpi::PE::instance(),ivec,3,ovec,mpi::plus );
  PEProcessSortedExecute(mpi::PE::instance(),-1,CFinfo << "Testing all_reduce " << ovec[0] << CFendl; );
  PEProcessSortedExecute(mpi::PE::instance(),-1,CFinfo << "Testing all_reduce " << ovec[1] << CFendl; );
  PEProcessSortedExecute(mpi::PE::instance(),-1,CFinfo << "Testing all_reduce " << ovec[2] << CFendl; );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( all_reduce )
{
  PEProcessSortedExecute(mpi::PE::instance(),-1,CFinfo << "Testing all_reduce " << irank << "/" << nproc << CFendl; );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( all_reduce_ptr_constant )
{
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( all_reduce_vector_constant )
{
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( all_reduce_ptr_variable )
{
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( all_reduce_vector_variable )
{
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
