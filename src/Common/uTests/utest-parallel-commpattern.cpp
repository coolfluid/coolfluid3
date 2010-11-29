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

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "Common/Log.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/MPI/PEObjectWrapper.hpp"
#include "Common/MPI/PECommPattern2.hpp"
#include "Common/CGroup.hpp"

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
  double *d2=new double[12];

  for(i=0; i<16; i++) d1[i]=16+i;
  for(i=0; i<12; i++) d2[i]=12+i;

  PEObjectWrapper *w1=new PEObjectWrapperPtr<double>("Array1",d1,16,2,true);
  PEObjectWrapper *w2=new PEObjectWrapperPtr<double>("Array2",d2,12,3,false);

  BOOST_CHECK_EQUAL( w1->needs_update() , true );
  BOOST_CHECK_EQUAL( w2->needs_update() , false );

  BOOST_CHECK_EQUAL( w1->is_data_type_Uint() , false );
  BOOST_CHECK_EQUAL( w2->is_data_type_Uint() , false );

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
  delete[] d2;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ObjectWrapperVector )
{
  int i,j;
  std::vector<double> d1(16);
  std::vector<double> d2(12);

  for(i=0; i<16; i++) d1[i]=16+i;
  for(i=0; i<12; i++) d2[i]=12+i;

  PEObjectWrapper *w1=new PEObjectWrapperVector<double>("Vector1",d1,2,true);
  PEObjectWrapper *w2=new PEObjectWrapperVector<double>("Vector2",d2,3,false);

  BOOST_CHECK_EQUAL( w1->needs_update() , true );
  BOOST_CHECK_EQUAL( w2->needs_update() , false );

  BOOST_CHECK_EQUAL( w1->is_data_type_Uint() , false );
  BOOST_CHECK_EQUAL( w2->is_data_type_Uint() , false );

  BOOST_CHECK_EQUAL( w1->size() , 8 );
  BOOST_CHECK_EQUAL( w2->size() , 4 );

  BOOST_CHECK_EQUAL( w1->stride() , 2 );
  BOOST_CHECK_EQUAL( w2->stride() , 3 );

  BOOST_CHECK_EQUAL( w1->size_of() , sizeof(double) );
  BOOST_CHECK_EQUAL( w2->size_of() , sizeof(double) );

  double *dtest1=(double*)w1->data();
  double *dtest2=(double*)w2->data();

  BOOST_CHECK_EQUAL( dtest1 , &d1[0] );
  BOOST_CHECK_EQUAL( dtest2 , &d2[0] );

  for(i=0; i<w1->size()*w1->stride(); i++) BOOST_CHECK_EQUAL( dtest1[i] , 16+i );
  for(i=0; i<w2->size()*w2->stride(); i++) BOOST_CHECK_EQUAL( dtest2[i] , 12+i );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ObjectWrapperVectorWeakPtr )
{
  int i,j;
  boost::shared_ptr< std::vector<double> > d1( new std::vector<double>(16) );
  boost::shared_ptr< std::vector<double> > d2( new std::vector<double>(12) );

  for(i=0; i<16; i++) (*d1)[i]=16+i;
  for(i=0; i<12; i++) (*d2)[i]=12+i;

  PEObjectWrapper *w1=new PEObjectWrapperVectorWeakPtr<double>("VectorWeakPtr1",d1,2,true);
  PEObjectWrapper *w2=new PEObjectWrapperVectorWeakPtr<double>("VectorWeakPtr2",d2,3,false);

  BOOST_CHECK_EQUAL( w1->needs_update() , true );
  BOOST_CHECK_EQUAL( w2->needs_update() , false );

  BOOST_CHECK_EQUAL( w1->is_data_type_Uint() , false );
  BOOST_CHECK_EQUAL( w2->is_data_type_Uint() , false );

  BOOST_CHECK_EQUAL( w1->size() , 8 );
  BOOST_CHECK_EQUAL( w2->size() , 4 );

  BOOST_CHECK_EQUAL( w1->stride() , 2 );
  BOOST_CHECK_EQUAL( w2->stride() , 3 );

  BOOST_CHECK_EQUAL( w1->size_of() , sizeof(double) );
  BOOST_CHECK_EQUAL( w2->size_of() , sizeof(double) );

  double *dtest1=(double*)w1->data();
  double *dtest2=(double*)w2->data();

  BOOST_CHECK_EQUAL( dtest1 , &(*d1)[0] );
  BOOST_CHECK_EQUAL( dtest2 , &(*d2)[0] );

  for(i=0; i<w1->size()*w1->stride(); i++) BOOST_CHECK_EQUAL( dtest1[i] , 16+i );
  for(i=0; i<w2->size()*w2->stride(); i++) BOOST_CHECK_EQUAL( dtest2[i] , 12+i );

  BOOST_CHECK_EQUAL( d1.use_count() , 1 );
  BOOST_CHECK_EQUAL( d2.use_count() , 1 );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( data_registration_related )
{

  PECommPattern2 pecp("CommPattern2");
  BOOST_CHECK_EQUAL( pecp.isUpToDate() , false );

  boost::shared_ptr< std::vector<double> > d1( new std::vector<double>(16) );
  boost::shared_ptr< std::vector<double> > d2( new std::vector<double>(12) );

  // register data to PECommPattern2
  pecp.insert<double>("VectorWeakPtr1",d1,2,true);
  pecp.insert<double>("VectorWeakPtr2",d2,3,true);

  // these are just dummies to see the selective iteration
  Component::Ptr dir1  ( new CGroup ( "dir1" ) );
  Component::Ptr dir2  ( new CGroup ( "dir2" ) );
  pecp.add_component( dir1 );
  pecp.add_component( dir2 );

  // count all child
  BOOST_CHECK_EQUAL( pecp.get_child_count() , 4 );

  // count recursively childs but only of type PEObjectWrapper
  //BOOST_CHECK_EQUAL( find_components_recursively<PEObjectWrapper>(pecp).size() , 2 );

  // iterate recursively childs but only of type PEObjectWrapper
  BOOST_FOREACH( PEObjectWrapper& pobj, find_components_recursively<PEObjectWrapper>(pecp) )
  {
    BOOST_CHECK_EQUAL( pobj.type_name() , "PEObjectWrapper" );
    BOOST_CHECK_EQUAL( pobj.size_of() , sizeof(double) );
    if (pobj.name()=="VectorWeakPtr1"){
      BOOST_CHECK_EQUAL( pobj.size() , 8 );
      BOOST_CHECK_EQUAL( pobj.stride() , 2 );
      BOOST_CHECK_EQUAL( pobj.data() , &(*d1)[0] );
    }
    if (pobj.name()=="VectorWeakPtr2"){
      BOOST_CHECK_EQUAL( pobj.size() , 4 );
      BOOST_CHECK_EQUAL( pobj.stride() , 3 );
      BOOST_CHECK_EQUAL( pobj.data() , &(*d2)[0] );
    }
  }

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( commpattern )
{

  // general conts in this routine
  const int nproc=PE::instance().size();
  const int irank=PE::instance().rank();

  // commpattern
  PECommPattern2 pecp("CommPattern2");

  // stupid global-reverse global indices
  std::vector<Uint> gid(nproc);
  for (int i=0; i<gid.size(); i++) gid[i]=(nproc*nproc-1)-(irank*nproc+i);

  // rank is built such that total scatter
  std::vector<int> rank(nproc);
  for (int i=0; i<gid.size(); i++) gid[i]=i;

  // three additional arrays for testing
  std::vector<int> v1(nproc);
  for(int i=0;i<v1.size();i++) v1[i]=irank*10000+i+100;
  pecp.insert("v1",v1,1,true);
  std::vector<int> v2(2*nproc);
  for(int i=0;i<v1.size();i++) v1[i]=irank*10000+(i/2)*100+i%2;
  pecp.insert("v2",v2,2,true);
  std::vector<int> v3(nproc);
  for(int i=0;i<v1.size();i++) v1[i]=irank;
  pecp.insert("v3",v3,1,false);

  //

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

