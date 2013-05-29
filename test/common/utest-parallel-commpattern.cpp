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
#define BOOST_TEST_MODULE "Test module for cf3::common 's parallel environment - part of testing the commpattern."

////////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

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

struct CommPatternFixture
{
  /// common setup for each test case
  CommPatternFixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~CommPatternFixture()
  {
  }

  /// function for setting up a gid & rank combo (with size of 6*nproc on each process)
  void setupGidAndRank(std::vector<Uint>& gid, std::vector<Uint>& rank)
  {
    // global indices and ranks, ordering: 0 1 2 ... 0 0 1 1 2 2 ... 0 0 0 1 1 1 2 2 2 ...
    int nproc=PE::Comm::instance().size();
    int irank=PE::Comm::instance().rank();
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

BOOST_FIXTURE_TEST_SUITE( CommPatternSuite, CommPatternFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  PE::Comm::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL( PE::Comm::instance().is_active() , true );
  CFinfo.setFilterRankZero(false);
  PEProcessSortedExecute(-1,CFinfo << "Proccess " << PE::Comm::instance().rank() << "/" << PE::Comm::instance().size() << " reports in." << CFendl;);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( data_registration_related )
{
  boost::shared_ptr<CommPattern> pecp_ptr = allocate_component<CommPattern>("CommPattern");
  CommPattern& pecp = *pecp_ptr;
  BOOST_CHECK_EQUAL( pecp.isUpToDate() , false );

  boost::shared_ptr< std::vector<double> > d1( new std::vector<double>(32) );
  boost::shared_ptr< std::vector<double> > d2( new std::vector<double>(24) );

  // register data to CommPattern
  pecp.insert<double>("VectorWeakPtr1",*d1,2,true);
  pecp.insert<double>("VectorWeakPtr2",*d2,3,true);

  // these are just dummies to see the selective iteration
  boost::shared_ptr<Component> dir1  ( allocate_component<Group> ( "dir1" ) );
  boost::shared_ptr<Component> dir2  ( allocate_component<Group> ( "dir2" ) );
  pecp.add_component( dir1 );
  pecp.add_component( dir2 );

  // count all child
  BOOST_CHECK_EQUAL( pecp.count_children() , 4u );

  // count recursively childs but only of type CommWrapper
  //BOOST_CHECK_EQUAL( find_components_recursively<CommWrapper>(pecp).size() , 2 );

  // iterate recursively childs but only of type CommWrapper
  BOOST_FOREACH( CommWrapper& pobj, find_components_recursively<CommWrapper>(pecp) )
  {
    BOOST_CHECK_EQUAL( pobj.type_name() , "CommWrapper" );
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

/// @todo see why doesnt work
//BOOST_AUTO_TEST_CASE( commpattern_cast )
//{
//  CommPattern pecp("CommPattern");
//  std::vector<Uint> gid(10);
//  pecp.insert("gid",gid);
//  CommWrapper globid=pecp.get_child("gid").as_type<CommPattern>();
//}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( commpattern_mainstream )
{
  // general constants in this routine
  const int nproc=PE::Comm::instance().size();
  const int irank=PE::Comm::instance().rank();

  // commpattern
  boost::shared_ptr<CommPattern> pecp_ptr = allocate_component<CommPattern>("CommPattern");
  CommPattern& pecp = *pecp_ptr;

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
  pecp.setup(Handle<CommWrapper>(pecp.get_child("gid")),rank);

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
/*
  // general constants in this routine
  const int nproc=PE::Comm::instance().size();
  const int irank=PE::Comm::instance().rank();

  // commpattern
  CommPattern pecp("CommPattern");

  // setup gid & rank
  std::vector<Uint> pre_gid; // it is used to feed through series of adds
  std::vector<Uint> gid(0);
  std::vector<Uint> rank;
  setupGidAndRank(pre_gid,rank);

PECheckPoint(1000,"001");
  pecp.insert("gid",gid,1,false);
PECheckPoint(1000,"002");

  // additional arrays for testing
  std::vector<int> v1;
  for(int i=0;i<6*nproc;i++) v1.push_back(-((irank+1)*1000+i+1));
  std::vector<double> v2;
  for(int i=0;i<12*nproc;i++) v2.push_back((double)((irank+1)*1000+i+1));

  // initial setup
  for (int i=0; i<gid.size(); i++) pecp.add(pre_gid[i],rank[i]);
PECheckPoint(1000,"003");
//THIS CRASHES,BECAUSE m_gid IS NOT SET IF THE ZERO ARGUMENT VERSION OF SETUP IS BEING CALLED
  pecp.setup();
PECheckPoint(1000,"004");

  PECheckPoint(100,"Before");
  PEProcessSortedExecute(-1,PEDebugVector(gid,gid.size()));
  PEProcessSortedExecute(-1,PEDebugVector(v1,v1.size()));
  PEProcessSortedExecute(-1,PEDebugVector(v2,v2.size()));

  // synchronize data
PECheckPoint(1000,"005");
  pecp.synchronize_all();
PECheckPoint(1000,"006");

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
*/
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

