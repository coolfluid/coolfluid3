// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Common 's parallel environment (PE) part"

#include <vector>
#include <boost/test/unit_test.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/thread.hpp>
#define  CHKPT(msg) PEInterface::instance().barrier();std::cout << PEInterface::instance().rank() << msg << std::flush; PEInterface::instance().barrier();boost::this_thread::sleep(boost::posix_time::milliseconds(200));


// IMPORTANT:
// run it both on 1 and many cores
// for example: mpirun -np 4 ./test-parallel-environment --report_level=confirm or --report_level=detailed

#include "Common/MPI/PEInterface.hpp"
#include "Common/MPI/PECommPattern.hpp"
#include "Common/MPI/all_to_all.hpp"
//#include "Common/MPI/scatterv.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

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
  
  /// communication pattern
  PECommPattern cp;

};

BOOST_FIXTURE_TEST_SUITE( PESuite, PEFixture )

// general tests on the environment

BOOST_AUTO_TEST_CASE( isinit_preinit )
{
  BOOST_CHECK_EQUAL( PEInterface::instance().is_init() , false );
}

BOOST_AUTO_TEST_CASE( allrankzero_preinit )
{
  BOOST_CHECK_EQUAL( PEInterface::instance().rank() , (Uint)0 );
}

BOOST_AUTO_TEST_CASE( init )
{
	PEInterface::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL( PEInterface::instance().is_init() , true );
}

BOOST_AUTO_TEST_CASE( rank_and_size )
{
  BOOST_CHECK_LT( PEInterface::instance().rank() , PEInterface::instance().size() );
}

// all to all communication tests
BOOST_AUTO_TEST_CASE( all_to_all )
{

  int i,j;
  const int nproc=PEInterface::instance().size();
  const int irank=PEInterface::instance().rank();

  // pointers
  int* ptr_sndcnt=new int[nproc];
  int* ptr_rcvcnt=new int[nproc];
  double* ptr_snddat=new double[nproc*nproc];
  double* ptr_rcvdat=new double[nproc*nproc];
  int* ptr_sndmap=new int[nproc*nproc];
  int* ptr_rcvmap=new int[nproc*nproc];
  double* ptr_tmprcv=0;
  //int* ptr_tmpcnt=new int[nproc];

  // setting up input and verification data
  for (i=0; i<nproc; i++)
    for (j=0; j<nproc; j++){
      ptr_snddat[i*nproc+j]=(irank+1)*1000000+(i+1)*10000+(j+1);
      ptr_rcvdat[i*nproc+j]=(i+1)*1000000+(irank+1)*10000+(j+1);
    }
  for (i=0; i<nproc; i++){
    ptr_sndcnt[i]=(i+irank*irank)%nproc;
    ptr_rcvcnt[i]=(i*i+irank)%nproc;
  }
  for (i=0; i<nproc; i++)
    for (j=0; j<ptr_sndcnt[i]; j++)
      ptr_sndmap[i]=nproc-1-j;
  for (i=0; i<nproc; i++)
    for (j=0; j<ptr_sndcnt[i]; j++)
      ptr_rcvmap[i]=nproc-1-j;


  // constant size - pointers
  ptr_tmprcv=0;

  ptr_tmprcv=(double*)mpi::all_to_all(PEInterface::instance(), ptr_snddat, nproc, (double*)0);
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[i] );

  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=0.;
  mpi::all_to_all(PEInterface::instance(), ptr_snddat, nproc, ptr_tmprcv);
  for (i=0; i<nproc*nproc; i++)  BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[i] );

  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=ptr_snddat[i];
  mpi::all_to_all(PEInterface::instance(), ptr_tmprcv, nproc, ptr_tmprcv);
  for (i=0; i<nproc*nproc; i++)  BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[i] );

  for (i=0; i<nproc*nproc; i++) ptr_tmprcv[i]=0.;
  mpi::all_to_all(PEInterface::instance(), (char*)ptr_snddat, nproc, (char*)ptr_tmprcv, sizeof(double));
  for (i=0; i<nproc*nproc; i++)  BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[i] );

  delete[] ptr_tmprcv;

  // variable size - pointers
/*
  ptr_tmprcv=0;
  ptr_tmprcv=(double*)mpi::all_to_all(PEInterface::instance(), ptr_snddat, ptr_sndcnt, (double*)0);
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[i] );
*/

  // std vectors
  std::vector<int> vec_sndcnt(nproc);
  vec_sndcnt.assign(ptr_sndcnt,ptr_sndcnt+nproc);
  std::vector<int> vec_rcvcnt(nproc);
  vec_rcvcnt.assign(ptr_rcvcnt,ptr_rcvcnt+nproc);
  std::vector<double> vec_snddat(nproc*nproc);
  vec_snddat.assign(ptr_snddat,ptr_snddat+nproc*nproc);
  std::vector<double> vec_rcvdat(nproc*nproc);
  vec_rcvdat.assign(ptr_rcvdat,ptr_rcvdat+nproc*nproc);
  std::vector<int> vec_sndmap(nproc*nproc);
  vec_sndmap.assign(ptr_sndmap,ptr_sndmap+nproc*nproc);
  std::vector<int> vec_rcvmap(nproc*nproc);
  vec_rcvmap.assign(ptr_rcvmap,ptr_rcvmap+nproc*nproc);
  std::vector<double> vec_tmprcv(0);

  // constant size std vectors
  vec_tmprcv.resize(0);
  vec_tmprcv.reserve(0);

  mpi::all_to_all(PEInterface::instance(), vec_snddat, vec_tmprcv);
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( vec_tmprcv[i] , vec_rcvdat[i] );
  BOOST_CHECK_EQUAL( vec_tmprcv.size() , nproc*nproc );

  vec_tmprcv.assign(nproc*nproc,0);
  mpi::all_to_all(PEInterface::instance(), vec_snddat, vec_tmprcv);
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( vec_tmprcv[i] , vec_rcvdat[i] );

  vec_tmprcv=vec_snddat;
  mpi::all_to_all(PEInterface::instance(), vec_tmprcv, vec_tmprcv);
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( vec_tmprcv[i] , vec_rcvdat[i] );

  std::vector<char> vec_tmprcvchr(0);
  vec_tmprcvchr.assign((char*)(&ptr_snddat[0]),(char*)(&ptr_snddat[nproc*nproc]));
  mpi::all_to_all(PEInterface::instance(), vec_tmprcvchr, vec_tmprcvchr );
  for (i=0; i<nproc*nproc; i++) BOOST_CHECK_EQUAL( ((double*)(&vec_tmprcvchr[0]))[i], vec_rcvdat[i] );

  vec_tmprcv.resize(0);
  vec_tmprcv.reserve(0);

  // free data
  delete[] ptr_sndcnt;
  delete[] ptr_rcvcnt;
  delete[] ptr_snddat;
  delete[] ptr_rcvdat;
  delete[] ptr_sndmap;
  delete[] ptr_rcvmap;
}

BOOST_AUTO_TEST_CASE( finalize )
{
  PEInterface::instance().finalize();
  BOOST_CHECK_EQUAL( PEInterface::instance().is_init() , false );
}

BOOST_AUTO_TEST_SUITE_END()
