// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
//

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Math::LSS where testing LSS::System and the dummy EmptyLSS."

////////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/assign/std/vector.hpp>

#include "Common/Log.hpp"
#include "Math/LSS/System.hpp"
#include "Common/MPI/CommPattern.hpp"
#include "Common/MPI/CommWrapper.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace CF;
using namespace CF::Math;
using namespace CF::Math::LSS;

using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

struct LSSSystem_EmptyLSSFixture
{
  /// common setup for each test case
  LSSSystem_EmptyLSSFixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~LSSSystem_EmptyLSSFixture()
  {
  }

  /// create a dummy commpattern
  void build_input_data()
  {
    cp = (Common::Comm::CommPattern::Ptr) new Common::Comm::CommPattern("commpattern");
    gid += 0,1,2,3,4,5,6,7,8,9;
    rnk += 0,0,0,0,0,0,0,0,0,0;
    conn += 0,2,1,2,2,7,3,13,4,14,5,2,6,0,7,12,8,10,9,8;  // so blocksize of the matrix will be 10x15
    startidx += 0,2,4,6,8,10,12,14,16,18,20;
    cp->insert("gid",gid,1,false);
    cp->setup(cp->get_child_ptr("gid")->as_ptr<Common::CommWrapper>(),rnk);
  }
  std::vector<Uint> gid;
  std::vector<Uint> conn;
  std::vector<Uint> startidx;
  std::vector<Uint> rnk;
  Common::Comm::CommPattern::Ptr cp;

  /// build a system
  void build_system()
  {
    s=(LSS::System::Ptr) new LSS::System("test_system");
    std::string solvertype("EmptyLSS");
    s->options().option("solver").change_value(solvertype);
    CFinfo << "HI: " << s->options().option("solver").value_str() << "\n";
    BOOST_CHECK_EQUAL(s->is_created(),false);
    s->create(*cp,4u,conn,startidx);
  }
  LSS::System::Ptr s;

  /// common params
  int m_argc;
  char** m_argv;
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( LSSSystem_EmptyLSSSuite, LSSSystem_EmptyLSSFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
  Common::Comm::PE::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL(Common::Comm::PE::instance().is_active(),true);
  CFinfo.setFilterRankZero(false);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( is_created )
{
  build_input_data();
  build_system();
  BOOST_CHECK_EQUAL(s->is_created(),true);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  CFinfo.setFilterRankZero(true);
  Common::Comm::PE::instance().finalize();
  BOOST_CHECK_EQUAL(Common::Comm::PE::instance().is_active(),false);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

