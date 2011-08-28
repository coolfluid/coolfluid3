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

  /// common params
  int m_argc;
  char** m_argv;
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( LSSSystem_EmptyLSSSuite, LSSSystem_EmptyLSSFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( hello_world )
{
/*
  Common::Comm::PE::instance().init(m_argc,m_argv);

  // create a dummy commpattern
  Common::Comm::CommPattern cp("dummy_commpattern");
  std::vector<Uint> gid;
  gid += 0,1,2,3,4,5,6,7,8,9;
  cp.insert("gid",gid,1,false);
  std::vector<Uint> rnk;
  rnk += 0,0,0,0,0,0,0,0,0,0;
  cp.setup(cp.get_child_ptr("gid")->as_ptr<Common::CommWrapper>(),rnk);

  // create a dummy connectivity
  std::vector<Uint> conn(0);
  std::vector<Uint> startidx(11,0);
*/
  // build a system
  LSS::System s("test_system");
  std::string solvertype("EmptyLSS");
  s.options().option("solver").change_value(solvertype);
  CFinfo << "HI: " << s.options().option("solver").value_str() << "\n";
  //s.is_created();
  //s.create(cp,4u,conn,startidx);

//  Common::Comm::PE::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

