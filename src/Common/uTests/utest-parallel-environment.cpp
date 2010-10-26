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
//#include <boost/thread/thread.hpp>

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
//PEInterface::instance().barrier();std::cout << PEInterface::instance().rank() << "00\n" << std::flush; PEInterface::instance().barrier();boost::this_thread::sleep(boost::posix_time::milliseconds(200));

  const int nproc=PEInterface::instance().size();

//PEInterface::instance().barrier();std::cout << PEInterface::instance().rank() << "01\n" << std::flush; PEInterface::instance().barrier();boost::this_thread::sleep(boost::posix_time::milliseconds(200));

  // send side and receive side for check
  int* ptr_sndcnt=new int[nproc];
  int* ptr_rcvcnt=new int[nproc];
  double* ptr_snddat=new double[nproc*nproc];
  double* ptr_rcvdat=new double[nproc*nproc];
  int* ptr_sndmap=new int[PEInterface::instance().size()*PEInterface::instance().size()];
  int* ptr_rcvmap=new int[PEInterface::instance().size()*PEInterface::instance().size()];

//PEInterface::instance().barrier();std::cout << PEInterface::instance().rank() << "02\n" << std::flush; PEInterface::instance().barrier();boost::this_thread::sleep(boost::posix_time::milliseconds(200));

  // constant size
  for (i=0; i<nproc; i++)
    for (j=0; j<nproc; j++){
      ptr_snddat[i*nproc+j]=i*10000+j;
      ptr_rcvdat[i*nproc+j]=j*10000+i;
std::cout << PEInterface::instance().rank() <<  "   " << ptr_snddat[i*nproc+j] << "   " << ptr_rcvdat[i*nproc+j] << "\n" << std::flush;
    }
//PEInterface::instance().barrier();std::cout << PEInterface::instance().rank() << "03\n" << std::flush; PEInterface::instance().barrier();boost::this_thread::sleep(boost::posix_time::milliseconds(200));
  double* ptr_tmprcv=(double*)mpi::all_to_all(PEInterface::instance(), ptr_snddat, nproc, (double*)0);
//PEInterface::instance().barrier();std::cout << PEInterface::instance().rank() << "04\n" << std::flush; PEInterface::instance().barrier();boost::this_thread::sleep(boost::posix_time::milliseconds(200));
  for (i=0; i<nproc*nproc; i++){
//      BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[i] );
std::cout << PEInterface::instance().rank() <<  "   " << ptr_tmprcv[i] << "   " << ptr_rcvdat[i] << "\n" << std::flush;
      ptr_tmprcv[i]=0.;
  }
//PEInterface::instance().barrier();std::cout << PEInterface::instance().rank() << "05\n" << std::flush; PEInterface::instance().barrier();boost::this_thread::sleep(boost::posix_time::milliseconds(200));
  mpi::all_to_all(PEInterface::instance(), ptr_snddat, nproc, ptr_tmprcv);
//PEInterface::instance().barrier();std::cout << PEInterface::instance().rank() << "06\n" << std::flush; PEInterface::instance().barrier();boost::this_thread::sleep(boost::posix_time::milliseconds(200));
  for (i=0; i<nproc*nproc; i++){
//      BOOST_CHECK_EQUAL( ptr_tmprcv[i] , ptr_rcvdat[i] );
  }
//PEInterface::instance().barrier();std::cout << PEInterface::instance().rank() << "07\n" << std::flush; PEInterface::instance().barrier();boost::this_thread::sleep(boost::posix_time::milliseconds(200));
  delete(ptr_tmprcv);
//PEInterface::instance().barrier();std::cout << PEInterface::instance().rank() << "08\n" << std::flush; PEInterface::instance().barrier();boost::this_thread::sleep(boost::posix_time::milliseconds(200));



  // free data
  delete(ptr_sndcnt);
  delete(ptr_rcvcnt);
  delete(ptr_snddat);
  delete(ptr_rcvdat);
  delete(ptr_sndmap);
  delete(ptr_rcvmap);
}


/*
BOOST_AUTO_TEST_CASE( collective_op )
{
  Uint rank_based_sum=0,size_based_sum=0;
  std::vector<Uint> ranklist(PEInterface::instance().size(),0);
  mpi::all_gather(PEInterface::instance(),PEInterface::instance().rank(),ranklist);
  for(Uint i=0; i<PEInterface::instance().size(); i++) {
    rank_based_sum+=ranklist[i];
    size_based_sum+=PEInterface::instance().size()-(i+1);
  }
  BOOST_CHECK_EQUAL( rank_based_sum , size_based_sum );
}

BOOST_AUTO_TEST_CASE( comm_pattern )
{
  // Do something with cp
}


BOOST_AUTO_TEST_CASE( scatterv )
{
 1 #include "boost/assign/std/vector.hpp"
 2 #include "boost/lambda/lambda.hpp"
 3
 4 using namespace boost::lambda;
 5 using namespace boost::assign;
 6 int main()
 7 {
 8   // use boost:assign to initialize the vector
 9   std::vector<int> y;
10   y += 1,2,3,4,5,6,7,8,9;
11
12   // use boost::lambda to pass a generic function to a for_each loop
13   for_each(y.begin(), y.end(), std::cout << _1 << '\n');
14 }
}
*/

BOOST_AUTO_TEST_CASE( finalize )
{
  PEInterface::instance().finalize();
  BOOST_CHECK_EQUAL( PEInterface::instance().is_init() , false );
}

BOOST_AUTO_TEST_SUITE_END()
