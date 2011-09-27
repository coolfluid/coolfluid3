// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
//
// IMPORTANT:
// run it both on 1 and many cores
// for example: mpirun -np 4 ./test-parallel-environment --report_level=confirm or --report_level=detailed

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Common 's parallel environment - part of testing the commwrapper."

////////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "Common/Log.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Component.hpp"
#include "Common/PE/Comm.hpp"
#include "Common/PE/CommWrapper.hpp"
#include "Common/PE/CommWrapperMArray.hpp"
#include "Common/PE/CommPattern.hpp"
#include "Common/PE/debug.hpp"
#include "Common/CGroup.hpp"


////////////////////////////////////////////////////////////////////////////////

using namespace CF;
using namespace CF::Common;
using namespace CF::Common::PE;

////////////////////////////////////////////////////////////////////////////////

struct CommWrapperFixture
{
  /// common setup for each test case
  CommWrapperFixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~CommWrapperFixture()
  {
  }

  /// common params
  int m_argc;
  char** m_argv;
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( CommWrapperSuite, CommWrapperFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  PE::Comm::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL( PE::Comm::instance().is_active() , true );
  CFinfo.setFilterRankZero(false);
  PEProcessSortedExecute(-1,CFinfo << "Proccess " << PE::Comm::instance().rank() << "/" << PE::Comm::instance().size() << " reports in." << CFendl;);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ObjectWrapperPtr )
{
  int i;
  Uint *d1=new Uint[16];
  double *d2=new double[24];
  std::vector<int> map(4);

  for(i=0; i<16; i++) d1[i]=16.+(double)i;
  for(i=0; i<24; i++) d2[i]=64.+(double)i;
  for(i=0; i<4; i+=2) map[i]=2+i;

  CommWrapperPtr<double>::Ptr w1=allocate_component< CommWrapperPtr<double> >("Ptr1");
  CommWrapperPtr<double>::Ptr w2=allocate_component< CommWrapperPtr<double> >("Ptr2");

  w1->setup(d1,16,1,true);
  w2->setup(d2,24,3,false);

  BOOST_CHECK_EQUAL( w1->needs_update() , true );
  BOOST_CHECK_EQUAL( w2->needs_update() , false );

  BOOST_CHECK_EQUAL( w1->is_data_type_Uint() , true );
  BOOST_CHECK_EQUAL( w2->is_data_type_Uint() , false );

  BOOST_CHECK_EQUAL( w1->size() , 16 );
  BOOST_CHECK_EQUAL( w2->size() , 8 );

  BOOST_CHECK_EQUAL( w1->stride() , 2 );
  BOOST_CHECK_EQUAL( w2->stride() , 3 );

  BOOST_CHECK_EQUAL( w1->size_of() , (int)sizeof(double) );
  BOOST_CHECK_EQUAL( w2->size_of() , (int)sizeof(double) );

  double *dtest1=(double*)w1->pack(map);
  double *dtest2=(double*)w2->pack(map);


/*
  int i;
  double *d1=new double[32];
  double *d2=new double[24];
  std::vector<int> map(4);

  for(i=0; i<32; i++) d1[i]=32.+(double)i;
  for(i=0; i<24; i++) d2[i]=64.+(double)i;
  for(i=0; i<4; i++) map[i]=2+i;

  CommWrapperPtr<double>::Ptr w1=allocate_component< CommWrapperPtr<double> >("Ptr1");
  CommWrapperPtr<double>::Ptr w2=allocate_component< CommWrapperPtr<double> >("Ptr2");

  w1->setup(d1,32,2,true);
  w2->setup(d2,24,3,false);

  BOOST_CHECK_EQUAL( w1->needs_update() , true );
  BOOST_CHECK_EQUAL( w2->needs_update() , false );

  BOOST_CHECK_EQUAL( w1->is_data_type_Uint() , false );
  BOOST_CHECK_EQUAL( w2->is_data_type_Uint() , false );

  BOOST_CHECK_EQUAL( w1->size() , 16 );
  BOOST_CHECK_EQUAL( w2->size() , 8 );

  BOOST_CHECK_EQUAL( w1->stride() , 2 );
  BOOST_CHECK_EQUAL( w2->stride() , 3 );

  BOOST_CHECK_EQUAL( w1->size_of() , (int)sizeof(double) );
  BOOST_CHECK_EQUAL( w2->size_of() , (int)sizeof(double) );

  double *dtest1=(double*)w1->pack(map);
  double *dtest2=(double*)w2->pack(map);

  for(i=0; i<8; i++) { BOOST_CHECK_EQUAL( dtest1[i] , 32+4+i ); dtest1[i]*=-1.; }
  for(i=0; i<12; i++) { BOOST_CHECK_EQUAL( dtest2[i] , 64+6+i ); dtest2[i]*=-1.; }

  w1->unpack(dtest1,map);
  w2->unpack(dtest2,map);

  double *dtesttest1=(double*)w1->pack(map);
  double *dtesttest2=(double*)w2->pack(map);

  for(i=0; i<8; i++) { BOOST_CHECK_EQUAL( dtesttest1[i] , -32-4-i ); }
  for(i=0; i<12; i++) { BOOST_CHECK_EQUAL( dtesttest2[i] , -64-6-i ); }

  double* dtesttesttest1=(double*)w1->pack();
  double* dtesttesttest2=(double*)w2->pack();

  for(i=0; i<8; i++) dtesttesttest1[4+i]*=-1.;
  for(i=0; i<12; i++) dtesttesttest2[6+i]*=-1.;

  w1->unpack(dtesttesttest1);
  w2->unpack(dtesttesttest2);

  double* dtesttesttesttest1=(double*)w1->pack();
  double* dtesttesttesttest2=(double*)w2->pack();

  for(i=0; i<32; i++) { BOOST_CHECK_EQUAL( dtesttesttesttest1[i] , 32+i ); }
  for(i=0; i<24; i++) { BOOST_CHECK_EQUAL( dtesttesttesttest2[i] , 64+i ); }

  delete[] dtest1;
  delete[] dtest2;
  delete[] dtesttest1;
  delete[] dtesttest2;
  delete[] dtesttesttest1;
  delete[] dtesttesttest2;
  delete[] dtesttesttesttest1;
  delete[] dtesttesttesttest2;
  delete[] d1;
  delete[] d2;
*/
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ObjectWrapperVector )
{

  int i;
  std::vector<double> d1(32);
  std::vector<double> d2(24);
  std::vector<int> map(4);

  for(i=0; i<32; i++) d1[i]=32.+(double)i;
  for(i=0; i<24; i++) d2[i]=64.+(double)i;
  for(i=0; i<4; i++) map[i]=2+i;

  CommWrapperVector<double>::Ptr w1=allocate_component< CommWrapperVector<double> >("Vector1");
  CommWrapperVector<double>::Ptr w2=allocate_component< CommWrapperVector<double> >("Vector2");

  w1->setup(d1,2,true);
  w2->setup(d2,3,false);

  BOOST_CHECK_EQUAL( w1->needs_update() , true );
  BOOST_CHECK_EQUAL( w2->needs_update() , false );

  BOOST_CHECK_EQUAL( w1->is_data_type_Uint() , false );
  BOOST_CHECK_EQUAL( w2->is_data_type_Uint() , false );

  BOOST_CHECK_EQUAL( w1->size() , 16 );
  BOOST_CHECK_EQUAL( w2->size() , 8 );

  BOOST_CHECK_EQUAL( w1->stride() , 2 );
  BOOST_CHECK_EQUAL( w2->stride() , 3 );

  BOOST_CHECK_EQUAL( w1->size_of() , (int)sizeof(double) );
  BOOST_CHECK_EQUAL( w2->size_of() , (int)sizeof(double) );

  double *dtest1=(double*)w1->pack(map);
  double *dtest2=(double*)w2->pack(map);

  for(i=0; i<8; i++) { BOOST_CHECK_EQUAL( dtest1[i] , 32+4+i ); dtest1[i]*=-1.; }
  for(i=0; i<12; i++) { BOOST_CHECK_EQUAL( dtest2[i] , 64+6+i ); dtest2[i]*=-1.; }

  w1->unpack(dtest1,map);
  w2->unpack(dtest2,map);

  double *dtesttest1=(double*)w1->pack(map);
  double *dtesttest2=(double*)w2->pack(map);

  for(i=0; i<8; i++) { BOOST_CHECK_EQUAL( dtesttest1[i] , -32-4-i ); }
  for(i=0; i<12; i++) { BOOST_CHECK_EQUAL( dtesttest2[i] , -64-6-i ); }

  double* dtesttesttest1=(double*)w1->pack();
  double* dtesttesttest2=(double*)w2->pack();

  for(i=0; i<8; i++) dtesttesttest1[4+i]*=-1.;
  for(i=0; i<12; i++) dtesttesttest2[6+i]*=-1.;

  w1->unpack(dtesttesttest1);
  w2->unpack(dtesttesttest2);

  double* dtesttesttesttest1=(double*)w1->pack();
  double* dtesttesttesttest2=(double*)w2->pack();

  for(i=0; i<32; i++) { BOOST_CHECK_EQUAL( dtesttesttesttest1[i] , 32+i ); }
  for(i=0; i<24; i++) { BOOST_CHECK_EQUAL( dtesttesttesttest2[i] , 64+i ); }

  delete[] dtest1;
  delete[] dtest2;
  delete[] dtesttest1;
  delete[] dtesttest2;
  delete[] dtesttesttest1;
  delete[] dtesttesttest2;
  delete[] dtesttesttesttest1;
  delete[] dtesttesttesttest2;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ObjectWrapperMultiArray )
{

  int i,j;
  boost::multi_array<Uint,1> array1d;
  boost::multi_array<Uint,2> array2d;

  array1d.resize(boost::extents[32]);
  array2d.resize(boost::extents[24][4]);

  std::vector<int> map(4);

  for(i=0; i<32; i++) array1d[i]=i;
  for(i=0; i<24; i++)
    for (j=0; j<4; j++)
      array2d[i][j]=i;
  for(i=0; i<4; i++) map[i]=2*i;

  CommWrapperMArray<Uint,1>::Ptr w1=allocate_component< CommWrapperMArray<Uint,1> >("array1d");
  CommWrapperMArray<Uint,2>::Ptr w2=allocate_component< CommWrapperMArray<Uint,2> >("array2d");

  CommWrapper::Ptr w3= build_component_abstract_type<CommWrapper>("CF.Common.CommWrapperMArray<unsigned,1>","array1d");
  CommWrapper::Ptr w4= build_component_abstract_type<CommWrapper>("CF.Common.CommWrapperMArray<unsigned,2>","array2d");

  w1->setup(array1d,true);
  w2->setup(array2d,false);

  BOOST_CHECK_EQUAL( w1->needs_update() , true );
  BOOST_CHECK_EQUAL( w2->needs_update() , false );

  BOOST_CHECK_EQUAL( w1->is_data_type_Uint() , true );
  BOOST_CHECK_EQUAL( w2->is_data_type_Uint() , true );

  BOOST_CHECK_EQUAL( w1->size() , 32 );
  BOOST_CHECK_EQUAL( w2->size() , 24 );

  BOOST_CHECK_EQUAL( w1->stride() , 1 );
  BOOST_CHECK_EQUAL( w2->stride() , 4 );

  BOOST_CHECK_EQUAL( w1->size_of() , (int)sizeof(Uint) );
  BOOST_CHECK_EQUAL( w2->size_of() , (int)sizeof(Uint) );

  Uint *dtest1=(Uint*)w1->pack(map);
  Uint *dtest2=(Uint*)w2->pack(map);

  for(i=0; i<4*1; i++) { BOOST_CHECK_EQUAL( dtest1[i] , (Uint) 2*i ); dtest1[i]+=1; }
  for(i=0; i<4*4; i++) { BOOST_CHECK_EQUAL( dtest2[i] , (Uint) 2*(i/4) ); dtest2[i]+=1; }

  w1->unpack(dtest1,map);
  w2->unpack(dtest2,map);

  Uint *dtesttest1=(Uint*)w1->pack(map);
  Uint *dtesttest2=(Uint*)w2->pack(map);

  for(i=0; i<4*1; i++) { BOOST_CHECK_EQUAL( dtesttest1[i] , (Uint) 2*i+1 ); }
  for(i=0; i<4*4; i++) { BOOST_CHECK_EQUAL( dtesttest2[i] , (Uint) 2*(i/4)+1 ); }

  Uint* dtesttesttest1=(Uint*)w1->pack();
  Uint* dtesttesttest2=(Uint*)w2->pack();

  for(i=0; i<32; i++) dtesttesttest1[i]=i+10;
  for(i=0; i<24; i++)
    for (j=0; j<4; j++)
      dtesttesttest2[i*4+j]=i+10;

  w1->unpack(dtesttesttest1);
  w2->unpack(dtesttesttest2);

  Uint* dtesttesttesttest1=(Uint*)w1->pack();
  Uint* dtesttesttesttest2=(Uint*)w2->pack();

  for(i=0; i<32; i++) { BOOST_CHECK_EQUAL( dtesttesttesttest1[i] , i+10 ); }
  for(i=0; i<24; i++)
    for (j=0; j<4; j++)
      { BOOST_CHECK_EQUAL( dtesttesttesttest2[i*4+j] , i+10 ); }

  delete[] dtest1;
  delete[] dtest2;
  delete[] dtesttest1;
  delete[] dtesttest2;
  delete[] dtesttesttest1;
  delete[] dtesttesttest2;
  delete[] dtesttesttesttest1;
  delete[] dtesttesttesttest2;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ObjectWrapperVectorWeakPtr )
{

  int i;
  boost::shared_ptr< std::vector<double> > d1( new std::vector<double>(32) );
  boost::shared_ptr< std::vector<double> > d2( new std::vector<double>(24) );
  std::vector<int> map(5);

  for(i=0; i<32; i++) (*d1)[i]=32.+(double)i;
  for(i=0; i<24; i++) (*d2)[i]=64.+(double)i;
  for(i=0; i<4; i++) map[i]=2+i;

  CommWrapperVectorWeakPtr<double>::Ptr w1=allocate_component< CommWrapperVectorWeakPtr<double> >("VectorWeakPtr1");
  CommWrapperVectorWeakPtr<double>::Ptr w2=allocate_component< CommWrapperVectorWeakPtr<double> >("VectorWeakPtr2");

  w1->setup(d1,2,true);
  w2->setup(d2,3,false);

  BOOST_CHECK_EQUAL( w1->needs_update() , true );
  BOOST_CHECK_EQUAL( w2->needs_update() , false );

  BOOST_CHECK_EQUAL( w1->is_data_type_Uint() , false );
  BOOST_CHECK_EQUAL( w2->is_data_type_Uint() , false );

  BOOST_CHECK_EQUAL( w1->size() , 16 );
  BOOST_CHECK_EQUAL( w2->size() , 8 );

  BOOST_CHECK_EQUAL( w1->stride() , 2 );
  BOOST_CHECK_EQUAL( w2->stride() , 3 );

  BOOST_CHECK_EQUAL( w1->size_of() , static_cast<int>(sizeof(double)) );
  BOOST_CHECK_EQUAL( w2->size_of() , static_cast<int>(sizeof(double)) );

  double *dtest1=(double*)w1->pack(map);
  double *dtest2=(double*)w2->pack(map);

  for(i=0; i<8; i++) { BOOST_CHECK_EQUAL( dtest1[i] , 32+4+i ); dtest1[i]*=-1.; }
  for(i=0; i<12; i++) { BOOST_CHECK_EQUAL( dtest2[i] , 64+6+i ); dtest2[i]*=-1.; }

  w1->unpack(dtest1,map);
  w2->unpack(dtest2,map);

  double *dtesttest1=(double*)w1->pack(map);
  double *dtesttest2=(double*)w2->pack(map);

  for(i=0; i<8; i++) { BOOST_CHECK_EQUAL( dtesttest1[i] , -32-4-i ); }
  for(i=0; i<12; i++) { BOOST_CHECK_EQUAL( dtesttest2[i] , -64-6-i ); }

  double* dtesttesttest1=(double*)w1->pack();
  double* dtesttesttest2=(double*)w2->pack();

  for(i=0; i<8; i++) dtesttesttest1[4+i]*=-1.;
  for(i=0; i<12; i++) dtesttesttest2[6+i]*=-1.;

  w1->unpack(dtesttesttest1);
  w2->unpack(dtesttesttest2);

  double* dtesttesttesttest1=(double*)w1->pack();
  double* dtesttesttesttest2=(double*)w2->pack();

  for(i=0; i<32; i++) { BOOST_CHECK_EQUAL( dtesttesttesttest1[i] , 32+i ); }
  for(i=0; i<24; i++) { BOOST_CHECK_EQUAL( dtesttesttesttest2[i] , 64+i ); }

  delete[] dtest1;
  delete[] dtest2;
  delete[] dtesttest1;
  delete[] dtesttest2;
  delete[] dtesttesttest1;
  delete[] dtesttesttest2;
  delete[] dtesttesttesttest1;
  delete[] dtesttesttesttest2;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize )
{
  PEProcessSortedExecute(-1,CFinfo << "Proccess " << PE::Comm::instance().rank() << "/" << PE::Comm::instance().size() << " says good bye." << CFendl;);
  CFinfo.setFilterRankZero(true);
  PE::Comm::instance().finalize();
  BOOST_CHECK_EQUAL( PE::Comm::instance().is_active() , false );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////


