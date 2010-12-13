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
#include "Common/Component.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/MPI/PEObjectWrapper.hpp"
#include "Common/MPI/PEObjectWrapperMultiArray.hpp"
#include "Common/MPI/PECommPattern.hpp"
#include "Common/MPI/tools.hpp"
#include "Common/CGroup.hpp"
#include "Common/CreateComponent.hpp"
/*
  TODO: move to allocate_component
*/

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

BOOST_AUTO_TEST_CASE( mpi_init )
{
  PE::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL( PE::instance().is_init() , true );
  PEProcessSortedExecute(PE::instance(),-1,CFinfo << "Proccess " << PE::instance().rank() << "/" << PE::instance().size() << " reports in." << CFendl;);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ObjectWrapperPtr )
{
  PEProcessSortedExecute(PE::instance(),-1,CFinfo << FromHere() << " " << PE::instance().rank() << "/" << PE::instance().size() << " reports in." << CFendl;);

  int i,j;
  double *d1=new double[32];
  double *d2=new double[24];
  std::vector<int> map(4);

  for(i=0; i<32; i++) d1[i]=32.+(double)i;
  for(i=0; i<24; i++) d2[i]=64.+(double)i;
  for(i=0; i<4; i++) map[i]=2+i;

  PEObjectWrapperPtr<double>::Ptr w1=allocate_component_type< PEObjectWrapperPtr<double> >("Ptr1");
  PEObjectWrapperPtr<double>::Ptr w2=allocate_component_type< PEObjectWrapperPtr<double> >("Ptr2");

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

  BOOST_CHECK_EQUAL( w1->size_of() , sizeof(double) );
  BOOST_CHECK_EQUAL( w2->size_of() , sizeof(double) );

  double *dtest1=(double*)w1->pack(map);
  double *dtest2=(double*)w2->pack(map);

  for(i=0; i<8; i++) { BOOST_CHECK_EQUAL( dtest1[i] , 32+4+i ); dtest1[i]*=-1.; }
  for(i=0; i<12; i++) { BOOST_CHECK_EQUAL( dtest2[i] , 64+6+i ); dtest2[i]*=-1.; }

  w1->unpack(map,dtest1);
  w2->unpack(map,dtest2);

  double *dtesttest1=(double*)w1->pack(map);
  double *dtesttest2=(double*)w2->pack(map);

  for(i=0; i<8; i++) { BOOST_CHECK_EQUAL( dtesttest1[i] , -32-4-i ); }
  for(i=0; i<12; i++) { BOOST_CHECK_EQUAL( dtesttest2[i] , -64-6-i ); }

  delete[] dtest1;
  delete[] dtest2;
  delete[] dtesttest1;
  delete[] dtesttest2;
  delete[] d1;
  delete[] d2;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ObjectWrapperVector )
{

  int i,j;
  std::vector<double> d1(32);
  std::vector<double> d2(24);
  std::vector<int> map(4);

  for(i=0; i<32; i++) d1[i]=32.+(double)i;
  for(i=0; i<24; i++) d2[i]=64.+(double)i;
  for(i=0; i<4; i++) map[i]=2+i;

  PEObjectWrapperVector<double>::Ptr w1=allocate_component_type< PEObjectWrapperVector<double> >("Vector1");
  PEObjectWrapperVector<double>::Ptr w2=allocate_component_type< PEObjectWrapperVector<double> >("Vector2");

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

  BOOST_CHECK_EQUAL( w1->size_of() , sizeof(double) );
  BOOST_CHECK_EQUAL( w2->size_of() , sizeof(double) );

  double *dtest1=(double*)w1->pack(map);
  double *dtest2=(double*)w2->pack(map);

  for(i=0; i<8; i++) { BOOST_CHECK_EQUAL( dtest1[i] , 32+4+i ); dtest1[i]*=-1.; }
  for(i=0; i<12; i++) { BOOST_CHECK_EQUAL( dtest2[i] , 64+6+i ); dtest2[i]*=-1.; }

  w1->unpack(map,dtest1);
  w2->unpack(map,dtest2);

  double *dtesttest1=(double*)w1->pack(map);
  double *dtesttest2=(double*)w2->pack(map);

  for(i=0; i<8; i++) { BOOST_CHECK_EQUAL( dtesttest1[i] , -32-4-i ); }
  for(i=0; i<12; i++) { BOOST_CHECK_EQUAL( dtesttest2[i] , -64-6-i ); }

  delete[] dtest1;
  delete[] dtest2;
  delete[] dtesttest1;
  delete[] dtesttest2;
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

  PEObjectWrapperMultiArray<Uint,1>::Ptr w1=allocate_component_type< PEObjectWrapperMultiArray<Uint,1> >("array1d");
  PEObjectWrapperMultiArray<Uint,2>::Ptr w2=allocate_component_type< PEObjectWrapperMultiArray<Uint,2> >("array2d");

  PEObjectWrapper::Ptr w3= create_component_abstract_type<PEObjectWrapper>("CF.Common.PEObjectWrapperMultiArray<unsigned,1>","array1d");
  PEObjectWrapper::Ptr w4= create_component_abstract_type<PEObjectWrapper>("CF.Common.PEObjectWrapperMultiArray<unsigned,2>","array2d");

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

  BOOST_CHECK_EQUAL( w1->size_of() , sizeof(Uint) );
  BOOST_CHECK_EQUAL( w2->size_of() , sizeof(Uint) );

  Uint *dtest1=(Uint*)w1->pack(map);
  Uint *dtest2=(Uint*)w2->pack(map);

  for(i=0; i<4*1; i++) { BOOST_CHECK_EQUAL( dtest1[i] , 2*i ); dtest1[i]+=1; }
  for(i=0; i<4*4; i++) { BOOST_CHECK_EQUAL( dtest2[i] , 2*(i/4) ); dtest2[i]+=1; }

  w1->unpack(map,dtest1);
  w2->unpack(map,dtest2);

  Uint *dtesttest1=(Uint*)w1->pack(map);
  Uint *dtesttest2=(Uint*)w2->pack(map);

  for(i=0; i<4*1; i++) { BOOST_CHECK_EQUAL( dtesttest1[i] , 2*i+1 ); }
  for(i=0; i<4*4; i++) { BOOST_CHECK_EQUAL( dtesttest2[i] , 2*(i/4)+1 ); }

  delete[] dtest1;
  delete[] dtest2;
  delete[] dtesttest1;
  delete[] dtesttest2;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ObjectWrapperVectorWeakPtr )
{

  int i,j;
  boost::shared_ptr< std::vector<double> > d1( new std::vector<double>(32) );
  boost::shared_ptr< std::vector<double> > d2( new std::vector<double>(24) );
  std::vector<int> map(5);

  for(i=0; i<32; i++) (*d1)[i]=32.+(double)i;
  for(i=0; i<24; i++) (*d2)[i]=64.+(double)i;
  for(i=0; i<4; i++) map[i]=2+i;

  PEObjectWrapperVectorWeakPtr<double>::Ptr w1=allocate_component_type< PEObjectWrapperVectorWeakPtr<double> >("VectorWeakPtr1");
  PEObjectWrapperVectorWeakPtr<double>::Ptr w2=allocate_component_type< PEObjectWrapperVectorWeakPtr<double> >("VectorWeakPtr2");

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

  BOOST_CHECK_EQUAL( w1->size_of() , sizeof(double) );
  BOOST_CHECK_EQUAL( w2->size_of() , sizeof(double) );

  double *dtest1=(double*)w1->pack(map);
  double *dtest2=(double*)w2->pack(map);

  for(i=0; i<8; i++) { BOOST_CHECK_EQUAL( dtest1[i] , 32+4+i ); dtest1[i]*=-1.; }
  for(i=0; i<12; i++) { BOOST_CHECK_EQUAL( dtest2[i] , 64+6+i ); dtest2[i]*=-1.; }

  w1->unpack(map,dtest1);
  w2->unpack(map,dtest2);

  double *dtesttest1=(double*)w1->pack(map);
  double *dtesttest2=(double*)w2->pack(map);

  for(i=0; i<8; i++) { BOOST_CHECK_EQUAL( dtesttest1[i] , -32-4-i ); }
  for(i=0; i<12; i++) { BOOST_CHECK_EQUAL( dtesttest2[i] , -64-6-i ); }

  delete[] dtest1;
  delete[] dtest2;
  delete[] dtesttest1;
  delete[] dtesttest2;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( data_registration_related )
{
  PECommPattern pecp("CommPattern2");
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
  BOOST_CHECK_EQUAL( pecp.get_child_count() , 4 );

  // count recursively childs but only of type PEObjectWrapper
  //BOOST_CHECK_EQUAL( find_components_recursively<PEObjectWrapper>(pecp).size() , 2 );

  // iterate recursively childs but only of type PEObjectWrapper
  BOOST_FOREACH( PEObjectWrapper& pobj, find_components_recursively<PEObjectWrapper>(pecp) )
  {
    BOOST_CHECK_EQUAL( pobj.type_name() , "PEObjectWrapper" );
    BOOST_CHECK_EQUAL( pobj.size_of() , sizeof(double) );
    if (pobj.name()=="VectorWeakPtr1"){
      BOOST_CHECK_EQUAL( pobj.size() , 16 );
      BOOST_CHECK_EQUAL( pobj.stride() , 2 );
    }
    if (pobj.name()=="VectorWeakPtr2"){
      BOOST_CHECK_EQUAL( pobj.size() , 8 );
      BOOST_CHECK_EQUAL( pobj.stride() , 3 );
    }
  }

const int nproc=PE::instance().size();
const int irank=PE::instance().rank();
PEProcessSortedExecute(PE::instance(),-1,CFinfo << "const int " << nproc << "/" << irank <<CFendl;);
int nproc_=PE::instance().size();
int irank_=PE::instance().rank();
PEProcessSortedExecute(PE::instance(),-1,CFinfo << "      int " << nproc_ << "/" << irank_ <<CFendl;);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( commpattern )
{
  // general conts in this routine
  const int nproc=PE::instance().size();
  const int irank=PE::instance().rank();

  // commpattern
  PECommPattern pecp("CommPattern2");

  // stupid global-reverse global indices
  std::vector<Uint> gid(nproc);
  for (int i=0; i<gid.size(); i++) gid[i]=(nproc*nproc-1)-(irank*nproc+i);
  pecp.insert("gid",gid,1,false);

PEProcessSortedExecute(PE::instance(),-1,PEDebugVector(gid,gid.size()));

/*
  // rank is built such that total scatter
  std::vector<int> rank(nproc);
  for (int i=0; i<gid.size(); i++) rank[i]=i;

  // three additional arrays for testing
  std::vector<int> v1;
  for(int i=0;i<nproc;i++) v1.push_back(irank*10000+i+100);
  pecp.insert("v1",v1,1,true);
  std::vector<int> v2;
  for(int i=0;i<2*nproc;i++) v2.push_back(irank*10000+(i/2)*100+i%2);
  pecp.insert("v2",v2,2,true);
  std::vector<int> v3;
  for(int i=0;i<nproc;i++) v3.push_back(irank);
  pecp.insert("v3",v3,1,false);

  // initial setup
//  pecp.setup(,rank);
*/
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize )
{
  PEProcessSortedExecute(PE::instance(),-1,CFinfo << "Proccess " << PE::instance().rank() << "/" << PE::instance().size() << " says good bye." << CFendl;);
  PE::instance().finalize();
  BOOST_CHECK_EQUAL( PE::instance().is_init() , false );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

