// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
//

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::math::LSS where testing indivdual operations."

////////////////////////////////////////////////////////////////////////////////

#include <fstream>

#include <boost/test/unit_test.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/lexical_cast.hpp>

#include "common/Log.hpp"
#include "math/LSS/System.hpp"
#include "math/VariablesDescriptor.hpp"

/// @todo remove when finished debugging
#include "common/PE/debug.hpp"
#include "common/Environment.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace boost::assign;

using namespace cf3;
using namespace cf3::math;
using namespace cf3::math::LSS;

////////////////////////////////////////////////////////////////////////////////

struct LSSSymmetricDirichletFixture
{
  /// common setup for each test case
  LSSSymmetricDirichletFixture() :
    solvertype("Trilinos"),
    gid(0),
    irank(0),
    nproc(1),
    neq(1),
    rank_updatable(0),
    node_connectivity(0),
    starting_indices(0),
    blockcol_size(0),
    blockrow_size(0)
  {
    if (common::PE::Comm::instance().is_initialized())
    {
      nproc=common::PE::Comm::instance().size();
      irank=common::PE::Comm::instance().rank();
      BOOST_CHECK_EQUAL(nproc,2);
    }
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;

    if(m_argc != 2)
      throw common::ParsingFailed(FromHere(), "Failed to parse command line arguments: expected one argument: builder name for the matrix");
    matrix_builder = m_argv[1];
  }

  /// common tear-down for each test case
  ~LSSSymmetricDirichletFixture()
  {
  }

  /// create a test commpattern
  void build_commpattern(common::PE::CommPattern& cp)
  {
    if (irank==0)
    {
      gid += 0,1,2;
      rank_updatable += 0,0,1;
    } else {
      gid += 1,2,3;
      rank_updatable += 0,1,1;
    }
    cp.insert("gid",gid,1,false);
    cp.setup(Handle<common::PE::CommWrapper>(cp.get_child("gid")),rank_updatable);
  }

  /// build a test system
  void build_system(LSS::System& sys, common::PE::CommPattern& cp)
  {
    if (irank==0)
    {
      node_connectivity += 0,1,0,1,2,1,2;
      starting_indices += 0,2,5,7;
      blockcol_size = 3;
      blockrow_size = 2;
    } else {
      node_connectivity += 0,1,0,1,2,1,2;
      starting_indices += 0,2,5,7;
      blockcol_size = 3;
      blockrow_size = 2;
    }
    sys.create(cp,neq,node_connectivity,starting_indices);
  }

  /// main solver selector
  std::string solvertype;
  std::string matrix_builder;

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
  int blockrow_size;
  int blockcol_size;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( LSSSymmetricDirichletSuite, LSSSymmetricDirichletFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
  common::PE::Comm::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL(common::PE::Comm::instance().is_active(),true);
  CFinfo.setFilterRankZero(false);
  common::Core::instance().environment().options().set("log_level", 4u);
  common::Core::instance().environment().options().set("exception_backtrace", false);
  common::Core::instance().environment().options().set("exception_outputs", false);
  //common::PE::wait_for_debugger(0);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_complete_system )
{
  // build a commpattern and the system
  boost::shared_ptr<common::PE::CommPattern> cp_ptr = common::allocate_component<common::PE::CommPattern>("commpattern");
  common::PE::CommPattern& cp = *cp_ptr;
  build_commpattern(cp);
  boost::shared_ptr<LSS::System> sys(common::allocate_component<LSS::System>("sys"));
  sys->options().option("matrix_builder").change_value(matrix_builder);
  build_system(*sys,cp);
  BOOST_CHECK_EQUAL(sys->is_created(),true);
  BOOST_CHECK_EQUAL(sys->solvertype(),solvertype);

  sys->matrix()->set_row(0, 0, 2, 1);
  sys->matrix()->set_row(1, 0, 2, 1);
  sys->matrix()->set_row(2, 0, 2, 1);

  if(irank == 0)
    sys->matrix()->symmetric_dirichlet(1, 0, 10., *sys->rhs());
  else
    sys->matrix()->symmetric_dirichlet(0, 0, 10., *sys->rhs());

  sys->rhs()->print_native(std::cout);

  Real val;
  if(irank == 0)
  {
    sys->matrix()->get_value(0, 0, val);
    BOOST_CHECK_EQUAL(val, 2.);
    sys->matrix()->get_value(1, 0, val);
    BOOST_CHECK_EQUAL(val, 0.);
    sys->matrix()->get_value(0, 1, val);
    BOOST_CHECK_EQUAL(val, 0.);
    sys->matrix()->get_value(1, 1, val);
    BOOST_CHECK_EQUAL(val, 1.);
    sys->matrix()->get_value(2, 1, val);
    BOOST_CHECK_EQUAL(val, 0.);

    sys->rhs()->get_value(0, val);
    BOOST_CHECK_EQUAL(val, -10.);
    sys->rhs()->get_value(1, val);
    BOOST_CHECK_EQUAL(val, 10.);
  }
  else
  {
    sys->matrix()->get_value(0, 1, val);
    BOOST_CHECK_EQUAL(val, 0.);
    sys->matrix()->get_value(1, 1, val);
    BOOST_CHECK_EQUAL(val, 2.);
    sys->matrix()->get_value(2, 1, val);
    BOOST_CHECK_EQUAL(val, 1.);
    sys->matrix()->get_value(1, 2, val);
    BOOST_CHECK_EQUAL(val, 1.);
    sys->matrix()->get_value(2, 2, val);
    BOOST_CHECK_EQUAL(val, 2.);

    sys->rhs()->get_value(0, val);
    BOOST_CHECK_EQUAL(val, 10.);
    sys->rhs()->get_value(1, val);
    BOOST_CHECK_EQUAL(val, -10.);
    sys->rhs()->get_value(2, val);
    BOOST_CHECK_EQUAL(val, 0.);
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  CFinfo.setFilterRankZero(true);
  common::PE::Comm::instance().finalize();
  BOOST_CHECK_EQUAL(common::PE::Comm::instance().is_active(),false);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

