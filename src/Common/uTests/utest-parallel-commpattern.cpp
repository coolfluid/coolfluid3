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
#define BOOST_TEST_MODULE "Test module for CF::Common 's parallel environment - part of testing the commpattern."

////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <boost/test/unit_test.hpp>

////////////////////////////////////////////////////////////////////////////////

#include "Common/MPI/PE.hpp"
#include "Common/MPI/PEObjectWrapper.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct PECommPatternFixture
{
  /// common setup for each test case
  PECommPatternFixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~PECommPatternFixture()
  {
    PE::instance().finalize();
  }

  /// common params
  int m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( PECommPatternSuite, PECommPatternFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  PE::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL( PE::instance().is_init() , true );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ObjectWrapperPtr )
{
  int i,j;
  double *d1=new double[16];
  double d2[12];

  for(i=0; i<16; i++) d1[i]=16+i;
  for(i=0; i<12; i++) d2[i]=12+i;

  PEObjectWrapper *w1=new PEObjectWrapperPtr<double>(d1,16,2);
  PEObjectWrapper *w2=new PEObjectWrapperPtr<double>(d2,12,3);

  BOOST_CHECK_EQUAL( w1->size() , 8 );
  BOOST_CHECK_EQUAL( w2->size() , 4 );

  BOOST_CHECK_EQUAL( w1->stride() , 2 );
  BOOST_CHECK_EQUAL( w2->stride() , 3 );

  BOOST_CHECK_EQUAL( w1->size_of() , sizeof(double) );
  BOOST_CHECK_EQUAL( w2->size_of() , sizeof(double) );

  double *dtest1=(double*)w1->data();
  double *dtest2=(double*)w2->data();

  BOOST_CHECK_EQUAL( dtest1 , d1 );
  BOOST_CHECK_EQUAL( dtest2 , d2 );

  for(i=0; i<w1->size()*w1->stride(); i++) BOOST_CHECK_EQUAL( dtest1[i] , 16+i );
  for(i=0; i<w2->size()*w2->stride(); i++) BOOST_CHECK_EQUAL( dtest2[i] , 12+i );

  delete[] d1;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ObjectWrapperVector )
{

  int i,j;
  std::vector<double> d1(16);
  std::vector<double> d2(12);

  for(i=0; i<16; i++) d1[i]=16+i;
  for(i=0; i<12; i++) d2[i]=12+i;

  PEObjectWrapper *w1=new PEObjectWrapperVector<double>(d1,2);
  PEObjectWrapper *w2=new PEObjectWrapperVector<double>(d2,3);
/*
  BOOST_CHECK_EQUAL( w1->size() , 8 );
  BOOST_CHECK_EQUAL( w2->size() , 4 );

  BOOST_CHECK_EQUAL( w1->stride() , 2 );
  BOOST_CHECK_EQUAL( w2->stride() , 3 );

  BOOST_CHECK_EQUAL( w1->size_of() , sizeof(double) );
  BOOST_CHECK_EQUAL( w2->size_of() , sizeof(double) );

  double *dtest1=(double*)w1->data();
  double *dtest2=(double*)w2->data();

std::cout << &d1[0] << "\n";

//  BOOST_CHECK_EQUAL( dtest1 , &d1[0] );
//  BOOST_CHECK_EQUAL( dtest2 , &d2[0] );

//  for(i=0; i<w1->size()*w1->stride(); i++) BOOST_CHECK_EQUAL( dtest1[i] , 16+i );
//  for(i=0; i<w2->size()*w2->stride(); i++) BOOST_CHECK_EQUAL( dtest2[i] , 12+i );
*/
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize )
{
  PE::instance().finalize();
  BOOST_CHECK_EQUAL( PE::instance().is_init() , false );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

