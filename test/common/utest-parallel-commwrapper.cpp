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
#define BOOST_TEST_MODULE "Test module for cf3::common 's parallel environment - part of testing the commwrapper."

////////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>

#include "common/Log.hpp"
#include "common/FindComponents.hpp"
#include "common/Component.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/CommWrapper.hpp"
#include "common/PE/CommWrapperMArray.hpp"
#include "common/PE/CommPattern.hpp"
#include "common/PE/debug.hpp"
#include "common/Group.hpp"


////////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::common;
using namespace cf3::common::PE;

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

  /// helper function to test setup
  void test_setup(Handle<CommWrapper>& w1,Handle<CommWrapper>& w2)
  {
    BOOST_CHECK_EQUAL( w1->needs_update() , true );
    BOOST_CHECK_EQUAL( w2->needs_update() , false );

    BOOST_CHECK_EQUAL( w1->is_data_type_Uint() , true );
    BOOST_CHECK_EQUAL( w2->is_data_type_Uint() , false );

    BOOST_CHECK_EQUAL( w1->size() , 16 );
    BOOST_CHECK_EQUAL( w2->size() , 8 );

    BOOST_CHECK_EQUAL( w1->stride() , 1 );
    BOOST_CHECK_EQUAL( w2->stride() , 3 );

    BOOST_CHECK_EQUAL( w1->size_of() , (int)sizeof(Uint) );
    BOOST_CHECK_EQUAL( w2->size_of() , (int)sizeof(double) );
  }

  /// helper function to test pack and unpack
  void test_pack_unpack(Handle<CommWrapper>& w1,Handle<CommWrapper>& w2)
  {
    int i;
    Uint   *itest1=(Uint*)  w1->pack();
    double *dtest2=(double*)w2->pack();

    for(i=0; i<16; i++) { BOOST_CHECK_EQUAL(itest1[i],16+i); itest1[i]+=16; }
    for(i=0; i<24; i++) { BOOST_CHECK_EQUAL(dtest2[i],24.+(double)i); dtest2[i]+=24.; }

    w1->unpack(itest1);
    w2->unpack(dtest2);

    for (i=0; i<16; i++) itest1[i]=0;
    for (i=0; i<24; i++) dtest2[i]=0.;

    w1->pack(itest1);
    w2->pack(dtest2);

    for(i=0; i<16; i++) { BOOST_CHECK_EQUAL(itest1[i],32+i); itest1[i]+=16; }
    for(i=0; i<24; i++) { BOOST_CHECK_EQUAL(dtest2[i],48.+(double)i); dtest2[i]+=24.; }

    std::vector<Uint> iv1; iv1.assign(itest1,itest1+16);
    std::vector<double> dv2; dv2.assign(dtest2,dtest2+24);

    w1->unpack(iv1);
    w2->unpack(dv2);

    iv1.assign(100,0);
    dv2.assign(100,0.);

    w1->pack(iv1);
    w2->pack(dv2);

    BOOST_CHECK_EQUAL(iv1.size(),16);
    BOOST_CHECK_EQUAL(dv2.size(),24);
    for(i=0; i<16; i++) { BOOST_CHECK_EQUAL(iv1[i],48+i); iv1[i]+=16; }
    for(i=0; i<24; i++) { BOOST_CHECK_EQUAL(dv2[i],72.+(double)i); dv2[i]+=24.; }

    std::vector<unsigned char> cv1; cv1.assign((unsigned char*)&iv1[0],(unsigned char*)&iv1[0]+16*sizeof(Uint));
    std::vector<unsigned char> cv2; cv2.assign((unsigned char*)&dv2[0],(unsigned char*)&dv2[0]+24*sizeof(double));

    w1->unpack(cv1);
    w2->unpack(cv2);

    cv1.assign(1000,0x00);
    cv2.assign(1000,0x00);

    w1->pack(cv1);
    w2->pack(cv2);

    BOOST_CHECK_EQUAL(cv1.size(),16*sizeof(Uint));
    BOOST_CHECK_EQUAL(cv2.size(),24*sizeof(double));
    for(i=0; i<16; i++) { BOOST_CHECK_EQUAL( ((Uint*)(&cv1[0]))[i] , 64+i ); ((Uint*)(&cv1[0]))[i]+=16; }
    for(i=0; i<24; i++) { BOOST_CHECK_EQUAL( ((double*)(&cv2[0]))[i] , (double)(96+i) ); ((double*)(&cv2[0]))[i]+=24.; }

    w1->unpack(cv1);
    w2->unpack(cv2);

    delete[] itest1;
    delete[] dtest2;

    w1->pack(iv1);
    w2->pack(dv2);

    for(i=0; i<16; i++) iv1[i]-=64;
    for(i=0; i<24; i++) dv2[i]-=96.;
    for(i=0; i<16; i++) BOOST_CHECK_EQUAL(iv1[i],16+i);
    for(i=0; i<24; i++) BOOST_CHECK_EQUAL(dv2[i],24.+(double)i);

    w1->unpack(iv1);
    w2->unpack(dv2);
  }

  /// helper function to test  mapped pack and unpack
  void test_mapped_pack_unpack(Handle<CommWrapper>& w1,Handle<CommWrapper>& w2, std::vector<int>&map)
  {
    int i,j;
    Uint   *itest1=(Uint*)  w1->pack(map);
    double *dtest2=(double*)w2->pack(map);

    for(i=0; i<4; i++) { BOOST_CHECK_EQUAL( itest1[i] , 16+map[i] ); itest1[i]+=16; }
    for(i=0; i<4; i++) for (j=0; j<3; j++) { BOOST_CHECK_EQUAL( dtest2[i*3+j] , (double)(24+3*map[i]+j) ); dtest2[i*3+j]+=24.; }

    w1->unpack(itest1,map);
    w2->unpack(dtest2,map);

    for (i=0; i<4; i++) itest1[i]=0;
    for (i=0; i<12; i++) dtest2[i]=0.;

    w1->pack(map,itest1);
    w2->pack(map,dtest2);

    for(i=0; i<4; i++) { BOOST_CHECK_EQUAL( itest1[i] , 32+map[i] ); itest1[i]+=16; }
    for(i=0; i<4; i++) for (j=0; j<3; j++) { BOOST_CHECK_EQUAL( dtest2[i*3+j] , (double)(48+3*map[i]+j) ); dtest2[i*3+j]+=24.; }

    std::vector<Uint> iv1; iv1.assign(itest1,itest1+4);
    std::vector<double> dv2; dv2.assign(dtest2,dtest2+12);

    w1->unpack(iv1,map);
    w2->unpack(dv2,map);

    iv1.assign(100,0);
    dv2.assign(100,0.);

    w1->pack(iv1,map);
    w2->pack(dv2,map);

    BOOST_CHECK_EQUAL(iv1.size(),4);
    BOOST_CHECK_EQUAL(dv2.size(),12);
    for(i=0; i<4; i++) { BOOST_CHECK_EQUAL( iv1[i] , 48+map[i] ); iv1[i]+=16; }
    for(i=0; i<4; i++) for (j=0; j<3; j++) { BOOST_CHECK_EQUAL( dv2[i*3+j] , (double)(72+3*map[i]+j) ); dv2[i*3+j]+=24.; }

    std::vector<unsigned char> cv1; cv1.assign((unsigned char*)&iv1[0],(unsigned char*)&iv1[0]+4*sizeof(Uint));
    std::vector<unsigned char> cv2; cv2.assign((unsigned char*)&dv2[0],(unsigned char*)&dv2[0]+12*sizeof(double));

    w1->unpack(cv1,map);
    w2->unpack(cv2,map);

    cv1.assign(1000,0x00);
    cv2.assign(1000,0x00);

    w1->pack(cv1,map);
    w2->pack(cv2,map);

    BOOST_CHECK_EQUAL(cv1.size(),4*sizeof(Uint));
    BOOST_CHECK_EQUAL(cv2.size(),12*sizeof(double));
    for(i=0; i<4; i++) { BOOST_CHECK_EQUAL( ((Uint*)(&cv1[0]))[i] , 64+map[i] ); ((Uint*)(&cv1[0]))[i]+=16; }
    for(i=0; i<4; i++) for (j=0; j<3; j++) { BOOST_CHECK_EQUAL( ((double*)(&cv2[0]))[i*3+j] , (double)(96+3*map[i]+j) ); ((double*)(&cv2[0]))[i*3+j]+=24.; }

    w1->unpack(cv1,map);
    w2->unpack(cv2,map);

    delete[] itest1;
    delete[] dtest2;

    w1->pack(iv1);
    w2->pack(dv2);

    for(i=0; i<4; i++) iv1[map[i]]-=64;
    for(i=0; i<4; i++) for (j=0; j<3; j++) dv2[3*map[i]+j]-=96.;
    for(i=0; i<16; i++) BOOST_CHECK_EQUAL(iv1[i],16+i);
    for(i=0; i<24; i++) BOOST_CHECK_EQUAL(dv2[i],24.+(double)i);

    w1->unpack(iv1);
    w2->unpack(dv2);
  }

  /// helper function for testing views
  void test_view(Handle<CommWrapper>& w1,Handle<CommWrapper>& w2)
  {
    int i;

    CommWrapperView<Uint> iwv1(w1);
    CommWrapperView<double> dwv2(w2);

    BOOST_CHECK_EQUAL(iwv1.size(),16);
    BOOST_CHECK_EQUAL(dwv2.size(),24);

    Uint* ip=iwv1.get_ptr(); // here get_ptr function is used
    double* dp=dwv2.get_ptr();

    for(i=0; i<16; i++) BOOST_CHECK_EQUAL(ip[i],16+i);
    for(i=0; i<24; i++) BOOST_CHECK_EQUAL(dp[i],24.+(double)i);

    CommWrapperView<unsigned char> cwv1(w1);
    CommWrapperView<unsigned char> cwv2(w2);

    BOOST_CHECK_EQUAL(cwv1.size(),16*sizeof(Uint));
    BOOST_CHECK_EQUAL(cwv2.size(),24*sizeof(double));

    Uint* ia=(Uint*)cwv1(); // here operator () is used
    double* da=(double*)cwv2();

    for(i=0; i<16; i++) BOOST_CHECK_EQUAL(ia[i],16+i);
    for(i=0; i<24; i++) BOOST_CHECK_EQUAL(da[i],24.+(double)i);
  }

  /// helper function for testing resize
  void test_resize(Handle<CommWrapper>& w1,Handle<CommWrapper>& w2)
  {
    int i;

    w1->resize(w1->size()+10);
    w2->resize(w2->size()+10);

    BOOST_CHECK_EQUAL( w1->size() , 26 );
    BOOST_CHECK_EQUAL( w2->size() , 18 );
    BOOST_CHECK_EQUAL( w1->stride() , 1 );
    BOOST_CHECK_EQUAL( w2->stride() , 3 );

    w1->resize(w1->size()-15);
    w2->resize(w2->size()-15);

    BOOST_CHECK_EQUAL( w1->size() , 11 );
    BOOST_CHECK_EQUAL( w2->size() , 3 );
    BOOST_CHECK_EQUAL( w1->stride() , 1 );
    BOOST_CHECK_EQUAL( w2->stride() , 3 );

    CommWrapperView<Uint> iwv1(w1);
    CommWrapperView<double> dwv2(w2);

    BOOST_CHECK_EQUAL(iwv1.size(),11);
    BOOST_CHECK_EQUAL(dwv2.size(),9);

    Uint* ia;
    double*da;
    for (ia=iwv1(), i=0; i<(const int)iwv1.size(); i++) BOOST_CHECK_EQUAL(ia[i],16+i);
    for (da=dwv2(), i=0; i<(const int)dwv2.size(); i++) BOOST_CHECK_EQUAL(da[i],24.+(double)i);
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
  Uint *i1=  new Uint[16];
  double *d2=new double[24];
  std::vector<int> map(4);

  for(i=0; i<16; i++) i1[i]=16+i;
  for(i=0; i<24; i++) d2[i]=24.+(double)i;
  for(i=0; i<4; i++) map[i]=1+2*i;

  boost::shared_ptr< CommWrapperPtr<Uint> > wptr1=  allocate_component< CommWrapperPtr<Uint>   >("Ptr1");
  boost::shared_ptr< CommWrapperPtr<double> > wptr2=allocate_component< CommWrapperPtr<double> >("Ptr2");
  Handle<CommWrapper> w1(wptr1);
  Handle<CommWrapper> w2(wptr2);

  wptr1->setup(i1,16,1,true);
  wptr2->setup(d2,24,3,false);

  test_setup(w1,w2);
  test_pack_unpack(w1,w2);
  test_mapped_pack_unpack(w1,w2,map);
  test_view(w1,w2);
  try
  {
    w1->resize(5);
  }
  catch (...)
  {
    BOOST_CHECK_EQUAL(w1->size(),5);
  }
  try
  {
    w1->resize(6);
  }
  catch (...)
  {
    BOOST_CHECK_EQUAL(w1->size(),6);
  }

  delete[] i1;
  delete[] d2;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ObjectWrapperVector )
{
  int i;
  std::vector<Uint> i1(16);
  std::vector<double> d2(24);
  std::vector<int> map(4);

  for(i=0; i<16; i++) i1[i]=16+i;
  for(i=0; i<24; i++) d2[i]=24.+(double)i;
  for(i=0; i<4; i++) map[i]=1+2*i;

  boost::shared_ptr< CommWrapperVector<Uint> > wptr1=  allocate_component< CommWrapperVector<Uint>   >("Ptr1");
  boost::shared_ptr< CommWrapperVector<double> > wptr2=allocate_component< CommWrapperVector<double> >("Ptr2");
  Handle<CommWrapper> w1(wptr1);
  Handle<CommWrapper> w2(wptr2);

  wptr1->setup(i1,1,true);
  wptr2->setup(d2,3,false);

  test_setup(w1,w2);
  test_pack_unpack(w1,w2);
  test_mapped_pack_unpack(w1,w2,map);
  test_view(w1,w2);
  test_resize(w1,w2);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ObjectWrapperMultiArray )
{
  int i,j;
  boost::multi_array<Uint,1> i1;
  boost::multi_array<double,2> d2;
  std::vector<int> map(4);
  i1.resize(boost::extents[16]);
  d2.resize(boost::extents[8][3]);

  for(i=0; i<16; i++) i1[i]=16+i;
  for(i=0; i<8; i++)
    for (j=0; j<3; j++)
      d2[i][j]=24.+(double)(3*i+j);
  for(i=0; i<4; i++) map[i]=1+2*i;

  boost::shared_ptr< CommWrapperMArray<Uint,1> > wptr1=  allocate_component< CommWrapperMArray<Uint,1>   >("Ptr1");
  boost::shared_ptr< CommWrapperMArray<double,2> > wptr2=allocate_component< CommWrapperMArray<double,2> >("Ptr2");
  Handle<CommWrapper> w1(wptr1);
  Handle<CommWrapper> w2(wptr2);

  wptr1->setup(i1,true);
  wptr2->setup(d2,false);

  test_setup(w1,w2);
  test_pack_unpack(w1,w2);
  test_mapped_pack_unpack(w1,w2,map);
  test_view(w1,w2);
  test_resize(w1,w2);
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


