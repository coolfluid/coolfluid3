// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
//

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Math::LSS where testing indivdual operations."

////////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/lexical_cast.hpp>

#include "Common/Log.hpp"
#include "Math/LSS/System.hpp"

/// @todo remove when finished debugging
#include "Common/MPI/debug.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace boost::assign;

using namespace CF;
using namespace CF::Math;
using namespace CF::Math::LSS;

////////////////////////////////////////////////////////////////////////////////

struct LSSAtomicFixture
{
  /// common setup for each test case
  LSSAtomicFixture() :
    gid(0),
    irank(0),
    nproc(1),
    neq(2),
    rank_updatable(0),
    node_connectivity(0),
    starting_indices(0)
  {
    if (Common::Comm::PE::instance().is_initialized())
    {
      nproc=Common::Comm::PE::instance().size();
      irank=Common::Comm::PE::instance().rank();
      BOOST_CHECK_EQUAL(nproc,2);
    }
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~LSSAtomicFixture()
  {
  }

  /// create a test commpattern
  void build_commpattern(Common::Comm::CommPattern& cp)
  {
    if (irank==0)
    {
      gid += 1,2,8,7,3,4,5,6;
      rank_updatable += 0,1,0,0,1,0,0,1;
    } else {
      gid += 5,0,2,7,1,3,8,4,6;
      rank_updatable += 0,1,1,0,0,1,0,0,1;
    }
    cp.insert("gid",gid,1,false);
    cp.setup(cp.get_child_ptr("gid")->as_ptr<Common::CommWrapper>(),rank_updatable);
  }

  /// build a test system
  void build_system(LSS::System::Ptr& sys, Common::Comm::CommPattern& cp)
  {
    if (irank==0)
    {
      node_connectivity += 0,2,4,6,1,2,3,1,3,5,7,1,2,3,5,0,1,3,4,5,6;
      starting_indices += 0,4,4,7,11,11,15,21,21;
    } else {
      node_connectivity += 1,2,5,0,2,3,4,5,7,1,5,6,2,6,8;
      starting_indices += 0,0,3,9,9,9,12,12,12,15;
    }
    sys->create(cp,neq,node_connectivity,starting_indices);
  }

  /// constructor builds
  int irank;
  int nproc;
  int neq;
  int m_argc;
  char** m_argv;

  /// commpattern builds
  std::vector<Uint> gid;
  std::vector<Uint> rank_updatable;

  /// system builds
  std::vector<Uint> node_connectivity;
  std::vector<Uint> starting_indices;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( LSSAtomicSuite, LSSAtomicFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
  Common::Comm::PE::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL(Common::Comm::PE::instance().is_active(),true);
  CFinfo.setFilterRankZero(false);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_matrix_only )
{
  // build a commpattern and a matrix
  Common::Comm::CommPattern cp("commpattern");
  build_commpattern(cp);
  LSS::System::Ptr sys(new LSS::System("sys"));
  sys->options().option("solver").change_value(boost::lexical_cast<std::string>("Trilinos"));
  build_system(sys,cp);
  BOOST_CHECK_EQUAL(sys->is_created(),true);

  // just to see if its crashing or not
  sys->print(std::cout);
  sys->print(CFinfo);
  std::string fname("test_matrix_" + boost::lexical_cast<std::string>(Common::Comm::PE::instance().rank()) + ".plt" );
  sys->print(fname);

  // entry-wise check

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

