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
#define BOOST_TEST_MODULE "Test module for CF::Common 's parallel environment - part of testing the commpattern."

////////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "Common/Log.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Component.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/MPI/PEObjectWrapper.hpp"
#include "Common/MPI/PEObjectWrapperMultiArray.hpp"
#include "Common/MPI/PECommPattern.hpp"
#include "Common/MPI/debug.hpp"
#include "Common/CGroup.hpp"
 

////////////////////////////////////////////////////////////////////////////////

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
  }

  /// function for setting up a gid & rank combo (with size of 6*nproc on each process)
  void setupGidAndRank(std::vector<Uint>& gid, std::vector<Uint>& rank)
  {
    // global indices and ranks, ordering: 0 1 2 ... 0 0 1 1 2 2 ... 0 0 0 1 1 1 2 2 2 ...
    int nproc=mpi::PE::instance().size();
    int irank=mpi::PE::instance().rank();
    gid.resize(6*nproc);
    rank.resize(6*nproc);
    for (Uint i=0; i<nproc; i++)
    {
      rank[0*nproc+1*i+0]=i;
      gid[ 0*nproc+1*i+0]=(6*nproc-1)-(6*i+0);
    }
    for (Uint i=0; i<nproc; i++)
    {
      rank[1*nproc+2*i+0]=i;
      rank[1*nproc+2*i+1]=i;
      gid[ 1*nproc+2*i+0]=(6*nproc-1)-(6*i+1);
      gid[ 1*nproc+2*i+1]=(6*nproc-1)-(6*i+2);
    }
    for (Uint i=0; i<nproc; i++)
    {
      rank[3*nproc+3*i+0]=i;
      rank[3*nproc+3*i+1]=i;
      rank[3*nproc+3*i+2]=i;
      gid[ 3*nproc+3*i+0]=(6*nproc-1)-(6*i+3);
      gid[ 3*nproc+3*i+1]=(6*nproc-1)-(6*i+4);
      gid[ 3*nproc+3*i+2]=(6*nproc-1)-(6*i+5);
    }
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
  mpi::PE::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL( mpi::PE::instance().is_active() , true );
  CFinfo.setFilterRankZero(false);
  PEProcessSortedExecute(-1,CFinfo << "Proccess " << mpi::PE::instance().rank() << "/" << mpi::PE::instance().size() << " reports in." << CFendl;);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ObjectWrapperPtr )
{
  int i;
  double *d1=new double[32];
  double *d2=new double[24];
  std::vector<int> map(4);

  for(i=0; i<32; i++) d1[i]=32.+(double)i;
  for(i=0; i<24; i++) d2[i]=64.+(double)i;
  for(i=0; i<4; i++) map[i]=2+i;

  PEObjectWrapperPtr<double>::Ptr w1=allocate_component< PEObjectWrapperPtr<double> >("Ptr1");
  PEObjectWrapperPtr<double>::Ptr w2=allocate_component< PEObjectWrapperPtr<double> >("Ptr2");

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

  PEObjectWrapperVector<double>::Ptr w1=allocate_component< PEObjectWrapperVector<double> >("Vector1");
  PEObjectWrapperVector<double>::Ptr w2=allocate_component< PEObjectWrapperVector<double> >("Vector2");

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

  PEObjectWrapperMultiArray<Uint,1>::Ptr w1=allocate_component< PEObjectWrapperMultiArray<Uint,1> >("array1d");
  PEObjectWrapperMultiArray<Uint,2>::Ptr w2=allocate_component< PEObjectWrapperMultiArray<Uint,2> >("array2d");

  PEObjectWrapper::Ptr w3= build_component_abstract_type<PEObjectWrapper>("CF.Common.PEObjectWrapperMultiArray<unsigned,1>","array1d");
  PEObjectWrapper::Ptr w4= build_component_abstract_type<PEObjectWrapper>("CF.Common.PEObjectWrapperMultiArray<unsigned,2>","array2d");

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

  PEObjectWrapperVectorWeakPtr<double>::Ptr w1=allocate_component< PEObjectWrapperVectorWeakPtr<double> >("VectorWeakPtr1");
  PEObjectWrapperVectorWeakPtr<double>::Ptr w2=allocate_component< PEObjectWrapperVectorWeakPtr<double> >("VectorWeakPtr2");

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

BOOST_AUTO_TEST_CASE( data_registration_related )
{
  PECommPattern pecp("CommPattern");
  BOOST_CHECK_EQUAL( pecp.isUpToDate() , false );

  boost::shared_ptr< std::vector<double> > d1( new std::vector<double>(32) );
  boost::shared_ptr< std::vector<double> > d2( new std::vector<double>(24) );

  // register data to PECommPattern
  pecp.insert<double>("VectorWeakPtr1",d1,2,true);
  pecp.insert<double>("VectorWeakPtr2",d2,3,true);

  // these are just dummies to see the selective iteration
  Component::Ptr dir1  ( new CGroup ( "dir1" ) );
  Component::Ptr dir2  ( new CGroup ( "dir2" ) );
  pecp.add_component( dir1 );
  pecp.add_component( dir2 );

  // count all child
  BOOST_CHECK_EQUAL( pecp.count_children() , 4u );

  // count recursively childs but only of type PEObjectWrapper
  //BOOST_CHECK_EQUAL( find_components_recursively<PEObjectWrapper>(pecp).size() , 2 );

  // iterate recursively childs but only of type PEObjectWrapper
  BOOST_FOREACH( PEObjectWrapper& pobj, find_components_recursively<PEObjectWrapper>(pecp) )
  {
    BOOST_CHECK_EQUAL( pobj.type_name() , "PEObjectWrapper" );
    BOOST_CHECK_EQUAL( pobj.size_of() , static_cast<int>(sizeof(double)) );
    if (pobj.name()=="VectorWeakPtr1"){
      BOOST_CHECK_EQUAL( pobj.size() , 16 );
      BOOST_CHECK_EQUAL( pobj.stride() , 2 );
    }
    if (pobj.name()=="VectorWeakPtr2"){
      BOOST_CHECK_EQUAL( pobj.size() , 8 );
      BOOST_CHECK_EQUAL( pobj.stride() , 3 );
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE( commpattern_cast )
{
  PECommPattern pecp("CommPattern");
  std::vector<Uint> gid(10);
  pecp.insert("gid",gid);
  PEObjectWrapper globid=pecp.get_child("gid").as_type<PECommPattern>();
}
*/
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( commpattern_mainstream )
{
  // general constants in this routine
  const int nproc=mpi::PE::instance().size();
  const int irank=mpi::PE::instance().rank();

  // commpattern
  PECommPattern pecp("CommPattern");

  // setup gid & rank
  std::vector<Uint> gid;
  std::vector<Uint> rank;
  setupGidAndRank(gid,rank);
  const int stride=1;
  const bool to_synchronize=false;
  pecp.insert("gid",gid,stride,to_synchronize);

  // additional arrays for testing
  std::vector<int> v1;
  for(int i=0;i<6*nproc;i++) v1.push_back(-((irank+1)*1000+i+1));
  pecp.insert("v1",v1,1,true);
  std::vector<double> v2;
  for(int i=0;i<12*nproc;i++) v2.push_back((double)((irank+1)*1000+i+1));
  pecp.insert("v2",v2,2,true);

  // initial setup
  pecp.setup(pecp.get_child_ptr("gid")->as_ptr<PEObjectWrapper>(),rank);

  PECheckPoint(100,"Before");
  PEProcessSortedExecute(-1,PEDebugVector(gid,gid.size()));
  PEProcessSortedExecute(-1,PEDebugVector(v1,v1.size()));
  PEProcessSortedExecute(-1,PEDebugVector(v2,v2.size()));

  // synchronize data
  pecp.synchronize_all();

  PECheckPoint(100,"After");
  PEProcessSortedExecute(-1,PEDebugVector(gid,gid.size()));
  PEProcessSortedExecute(-1,PEDebugVector(v1,v1.size()));
  PEProcessSortedExecute(-1,PEDebugVector(v2,v2.size()));

  // check results
  Uint idx=0;
  Uint i;
  for (i=0; i<  nproc; i++, idx++ ) BOOST_CHECK_EQUAL( v1[i], (int)(-((((i-0*nproc)/1)+1)*1000+idx+1)) );
  for (   ; i<3*nproc; i++, idx++ ) BOOST_CHECK_EQUAL( v1[i], (int)(-((((i-1*nproc)/2)+1)*1000+idx+1)) );
  for (   ; i<6*nproc; i++, idx++ ) BOOST_CHECK_EQUAL( v1[i], (int)(-((((i-3*nproc)/3)+1)*1000+idx+1)) );
  idx=0;
  for (i=0; i< 2*nproc; i++, idx++) BOOST_CHECK_EQUAL( v2[i], (double)((((i-0*nproc)/2)+1)*1000+idx+1) );
  for (   ; i< 6*nproc; i++, idx++) BOOST_CHECK_EQUAL( v2[i], (double)((((i-2*nproc)/4)+1)*1000+idx+1) );
  for (   ; i<12*nproc; i++, idx++) BOOST_CHECK_EQUAL( v2[i], (double)((((i-6*nproc)/6)+1)*1000+idx+1) );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( commpattern_external_synchronization )
{
  // general constants in this routine
  const int nproc=mpi::PE::instance().size();
  const int irank=mpi::PE::instance().rank();

  // commpattern
  PECommPattern pecp("CommPattern");

  // setup gid & rank
  std::vector<Uint> pre_gid; // it is used to feed through series of adds
  std::vector<Uint> gid(0);
  std::vector<Uint> rank;
  setupGidAndRank(pre_gid,rank);
  pecp.insert("gid",gid,1,false);

  // additional arrays for testing
  std::vector<int> v1;
  for(int i=0;i<6*nproc;i++) v1.push_back(-((irank+1)*1000+i+1));
  std::vector<double> v2;
  for(int i=0;i<12*nproc;i++) v2.push_back((double)((irank+1)*1000+i+1));

  // initial setup
  for (int i=0; i<gid.size(); i++) pecp.add(pre_gid[i],rank[i]);
  pecp.setup();

  PECheckPoint(100,"Before");
  PEProcessSortedExecute(-1,PEDebugVector(gid,gid.size()));
  PEProcessSortedExecute(-1,PEDebugVector(v1,v1.size()));
  PEProcessSortedExecute(-1,PEDebugVector(v2,v2.size()));

  // synchronize data
  pecp.synchronize_all();

  PECheckPoint(100,"After");
  PEProcessSortedExecute(-1,PEDebugVector(gid,gid.size()));
  PEProcessSortedExecute(-1,PEDebugVector(v1,v1.size()));
  PEProcessSortedExecute(-1,PEDebugVector(v2,v2.size()));

  // check results
  Uint idx=0;
  Uint i;
  for (i=0; i<  nproc; i++, idx++ ) BOOST_CHECK_EQUAL( v1[i], (int)(-((((i-0*nproc)/1)+1)*1000+idx+1)) );
  for (   ; i<3*nproc; i++, idx++ ) BOOST_CHECK_EQUAL( v1[i], (int)(-((((i-1*nproc)/2)+1)*1000+idx+1)) );
  for (   ; i<6*nproc; i++, idx++ ) BOOST_CHECK_EQUAL( v1[i], (int)(-((((i-3*nproc)/3)+1)*1000+idx+1)) );
  idx=0;
  for (i=0; i< 2*nproc; i++, idx++) BOOST_CHECK_EQUAL( v2[i], (double)((((i-0*nproc)/2)+1)*1000+idx+1) );
  for (   ; i< 6*nproc; i++, idx++) BOOST_CHECK_EQUAL( v2[i], (double)((((i-2*nproc)/4)+1)*1000+idx+1) );
  for (   ; i<12*nproc; i++, idx++) BOOST_CHECK_EQUAL( v2[i], (double)((((i-6*nproc)/6)+1)*1000+idx+1) );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize )
{
  PEProcessSortedExecute(-1,CFinfo << "Proccess " << mpi::PE::instance().rank() << "/" << mpi::PE::instance().size() << " says good bye." << CFendl;);
  CFinfo.setFilterRankZero(true);
  mpi::PE::instance().finalize();
  BOOST_CHECK_EQUAL( mpi::PE::instance().is_active() , false );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

