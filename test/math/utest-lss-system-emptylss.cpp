// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
//

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::math::LSS where testing LSS::System and the dummy EmptyLSS."

////////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/assign/std/vector.hpp>

#include "common/Log.hpp"
#include "math/LSS/System.hpp"
#include "common/PE/CommPattern.hpp"
#include "common/PE/CommWrapper.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::math;
using namespace cf3::math::LSS;

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
    cp = common::allocate_component<common::PE::CommPattern>("commpattern");
    gid += 0,1,2,3,4,5,6,7,8,9;
    rnk += 0,0,0,0,0,0,0,0,0,0;
    conn += 0,2,1,2,2,7,3,8,4,5,5,2,6,0,7,1,8,7,9,8;
    startidx += 0,2,4,6,8,10,12,14,16,18,20;
    cp->insert("gid",gid,1,false);
    cp->setup(Handle<common::PE::CommWrapper>(cp->get_child("gid")),rnk);
  }
  std::vector<Uint> gid;
  std::vector<Uint> conn;
  std::vector<Uint> startidx;
  std::vector<Uint> rnk;
  boost::shared_ptr<common::PE::CommPattern> cp;

  /// build a system
  void build_system()
  {
    sys = common::allocate_component<LSS::System>("system");
    std::string solvertype("cf3.math.LSS.EmptyLSSMatrix");
    sys->options().option("matrix_builder").change_value(solvertype);
    BOOST_CHECK_EQUAL(sys->is_created(),false);
    sys->create(*cp,4u,conn,startidx);
    ba.resize(2,4);
    diag.resize(20,5.);
  }
  boost::shared_ptr<LSS::System> sys;
  LSS::BlockAccumulator ba;
  Real testval;
  std::vector<Real> diag;

  /// common params
  int m_argc;
  char** m_argv;
};

////////////////////////////////////////////////////////////////////////////////

#define CHECK_BLOCKACCUMULATOR_IS_CONSTANT(ba,refval) { \
  BOOST_CHECK_EQUAL(ba.mat.isConstant(refval,1.e-10),true); \
  Real sum=ba.mat.sum(); \
  Real numentries=(Real)(ba.mat.rows()*ba.mat.cols()); \
  BOOST_CHECK_CLOSE(sum,numentries*refval,1.e-10); \
  BOOST_CHECK_EQUAL(ba.rhs.isConstant(refval,1.e-10),true); \
  sum=ba.rhs.sum(); \
  numentries=(Real)(ba.rhs.rows()*ba.rhs.cols()); \
  BOOST_CHECK_CLOSE(sum,numentries*refval,1.e-10); \
  BOOST_CHECK_EQUAL(ba.sol.isConstant(refval,1.e-10),true); \
  sum=ba.sol.sum(); \
  numentries=(Real)(ba.sol.rows()*ba.sol.cols()); \
  BOOST_CHECK_CLOSE(sum,numentries*refval,1.e-10); \
}

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( LSSSystem_EmptyLSSSuite, LSSSystem_EmptyLSSFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
  common::PE::Comm::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL(common::PE::Comm::instance().is_active(),true);
  CFinfo.setFilterRankZero(false);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( is_created )
{
  build_input_data();
  build_system();
  // redundant check of state of creation
  BOOST_CHECK_EQUAL(sys->matrix()->is_created(),true);
  BOOST_CHECK_EQUAL(sys->rhs()->is_created(),true);
  BOOST_CHECK_EQUAL(sys->solution()->is_created(),true);
  BOOST_CHECK_EQUAL(sys->is_created(),true);
  sys->destroy();
  BOOST_CHECK_EQUAL(sys->is_created(),false);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( check_properties )
{
  build_input_data();
  build_system();
  BOOST_CHECK_EQUAL(sys->matrix()->blockrow_size(),10);
  BOOST_CHECK_EQUAL(sys->matrix()->blockcol_size(),10);
  BOOST_CHECK_EQUAL(sys->matrix()->neq(),4);
  BOOST_CHECK_EQUAL(sys->solution()->blockrow_size(),10);
  BOOST_CHECK_EQUAL(sys->solution()->neq(),4);
  BOOST_CHECK_EQUAL(sys->rhs()->blockrow_size(),10);
  BOOST_CHECK_EQUAL(sys->rhs()->neq(),4);
  BOOST_CHECK_EQUAL(sys->matrix()->solvertype(),"EmptyLSS");
  BOOST_CHECK_EQUAL(sys->solution()->solvertype(),"EmptyLSS");
  BOOST_CHECK_EQUAL(sys->rhs()->solvertype(),"EmptyLSS");
  BOOST_CHECK_EQUAL(sys->solvertype(),"EmptyLSS");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( blockaccumulator )
{
  ba.resize(2,4);
  ba.reset(1.0);
  CHECK_BLOCKACCUMULATOR_IS_CONSTANT(ba,1.);
  ba.reset();
  CHECK_BLOCKACCUMULATOR_IS_CONSTANT(ba,0.);
  BOOST_CHECK_EQUAL(ba.mat.rows(),8);
  BOOST_CHECK_EQUAL(ba.mat.cols(),8);
  BOOST_CHECK_EQUAL(ba.rhs.rows(),8);
  BOOST_CHECK_EQUAL(ba.rhs.cols(),1);
  BOOST_CHECK_EQUAL(ba.sol.rows(),8);
  BOOST_CHECK_EQUAL(ba.sol.cols(),1);

  ba.resize(5,2);
  ba.reset(1.0);
  CHECK_BLOCKACCUMULATOR_IS_CONSTANT(ba,1.);
  ba.reset();
  CHECK_BLOCKACCUMULATOR_IS_CONSTANT(ba,0.);
  BOOST_CHECK_EQUAL(ba.mat.rows(),10);
  BOOST_CHECK_EQUAL(ba.mat.cols(),10);
  BOOST_CHECK_EQUAL(ba.rhs.rows(),10);
  BOOST_CHECK_EQUAL(ba.rhs.cols(),1);
  BOOST_CHECK_EQUAL(ba.sol.rows(),10);
  BOOST_CHECK_EQUAL(ba.sol.cols(),1);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( check_emptylss_implementations )
{
  // this test case just calls functions to check if emptylss is implemented in a non-crashing way
  build_input_data();
  build_system();

  BOOST_TEST_CHECKPOINT( "get_values" );
  ba.reset(1.0);
  sys->get_values(ba);
  CHECK_BLOCKACCUMULATOR_IS_CONSTANT(ba,0.);

  BOOST_TEST_CHECKPOINT( "set_values" );
  sys->set_values(ba);

  BOOST_TEST_CHECKPOINT( "add_values" );
  sys->add_values(ba);

  BOOST_TEST_CHECKPOINT( "dirichlet" );
  sys->dirichlet(0,0,0.,true);

  BOOST_TEST_CHECKPOINT( "periodicity" );
  sys->periodicity (0,0);

  BOOST_TEST_CHECKPOINT( "get_diagonal" );
  sys->get_diagonal(diag);
  BOOST_CHECK_EQUAL(diag.size(),sys->matrix()->blockrow_size()*sys->matrix()->neq());
  testval=0.;
  BOOST_FOREACH(Real i, diag) testval+=i;
  BOOST_CHECK_CLOSE(testval,0., 1.e-10);

  BOOST_TEST_CHECKPOINT( "set_diagonal" );
  sys->set_diagonal(diag);

  BOOST_TEST_CHECKPOINT( "add_diagonal" );
  sys->add_diagonal(diag);

  BOOST_TEST_CHECKPOINT( "reset" );
  sys->reset(0.);

  BOOST_TEST_CHECKPOINT( "print" );
  sys->print(CFinfo);

  BOOST_TEST_CHECKPOINT( "matrix::get_value" );
  testval=1.;
  sys->matrix()->get_value(0,0,testval);
  BOOST_CHECK_EQUAL(testval,0.);

  BOOST_TEST_CHECKPOINT( "matrix::set_value" );
  sys->matrix()->set_value(0,0,testval);

  BOOST_TEST_CHECKPOINT( "matrix::add_value" );
  sys->matrix()->add_value(0,0,testval);

  BOOST_TEST_CHECKPOINT( "rhs::get_value" );
  testval=1.;
  sys->rhs()->get_value(0,testval);
  BOOST_CHECK_EQUAL(testval,0.);

  BOOST_TEST_CHECKPOINT( "rhs::set_value" );
  sys->rhs()->set_value(0,testval);

  BOOST_TEST_CHECKPOINT( "rhs::add_value" );
  sys->rhs()->add_value(0,testval);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( swap_matrix_vector )
{
  build_input_data();
  build_system();

  boost::shared_ptr<LSS::System> sys2 = common::allocate_component<LSS::System>("system2");
  std::string solvertype("cf3.math.LSS.EmptyLSSMatrix");
  sys2->options().option("matrix_builder").change_value(solvertype);
  sys2->create(*cp,4u,conn,startidx);
  sys2->matrix()->rename("Matrix2");
  sys2->rhs()->rename("RHS2");
  sys2->solution()->rename("Solution2");

  boost::shared_ptr<Matrix> sw_mat = boost::dynamic_pointer_cast<Matrix>(sys2->remove_component("Matrix2"));
  boost::shared_ptr<Vector> sw_sol = boost::dynamic_pointer_cast<Vector>(sys2->remove_component("Solution2"));
  boost::shared_ptr<Vector> sw_rhs = boost::dynamic_pointer_cast<Vector>(sys2->remove_component("RHS2"));
  
  sys->swap(sw_mat, sw_sol, sw_rhs);
  BOOST_CHECK_EQUAL(sys->matrix()->name(),"Matrix2");
  BOOST_CHECK_EQUAL(sys->solution()->name(),"Solution2");
  BOOST_CHECK_EQUAL(sys->rhs()->name(),"RHS2");
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


