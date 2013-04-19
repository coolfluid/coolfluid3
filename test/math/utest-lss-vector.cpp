// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
//

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for some LSS vector ops"

////////////////////////////////////////////////////////////////////////////////

#include <fstream>

#include <boost/test/unit_test.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/lexical_cast.hpp>

#include "common/Log.hpp"
#include "math/LSS/System.hpp"
#include "math/LSS/Trilinos/ThyraVector.hpp"
#include <math/LSS/Trilinos/ThyraOperator.hpp>
#include <math/LSS/Trilinos/TrilinosVector.hpp>
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

struct LSSAtomicFixture
{
  /// common setup for each test case
  LSSAtomicFixture() :
    solvertype("Trilinos"),
    gid(0),
    irank(0),
    nproc(1),
    neq(1),
    rank_updatable(0),
    node_connectivity(0),
    starting_indices(0)
  {
    if (common::PE::Comm::instance().is_initialized())
    {
      nproc=common::PE::Comm::instance().size();
      irank=common::PE::Comm::instance().rank();
      BOOST_CHECK_EQUAL(nproc,2);
    }
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;

    matrix_builder = "cf3.math.LSS.TrilinosFEVbrMatrix";
  }

  /// common tear-down for each test case
  ~LSSAtomicFixture()
  {
  }

  /// create a test commpattern
  void build_commpattern(common::PE::CommPattern& cp)
  {
    if (irank==0)
    {
      gid += 0,1,2,6;
      rank_updatable += 0,0,0,1;
    } else {
      gid += 0,3,4,5,6;
      rank_updatable += 0,1,1,1,1;
    }
    cp.insert("gid",gid,1,false);
    cp.setup(Handle<common::PE::CommWrapper>(cp.get_child("gid")),rank_updatable);
  }

  /// build a test system
  void build_system(LSS::System& sys, common::PE::CommPattern& cp)
  {
    starting_indices.clear();
    node_connectivity.clear();
    if (irank==0)
    {
      node_connectivity += 0,3,1,3,2,3,3;
      starting_indices += 0,2,4,6,7;
    } else {
      node_connectivity += 0,4,1,4,2,4,3,4,4;
      starting_indices +=  0,2,4,6,8,9;
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

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( LSSAtomicSuite, LSSAtomicFixture )

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

BOOST_AUTO_TEST_CASE( test_thyra_convert )
{
  boost::shared_ptr<common::PE::CommPattern> cp_ptr = common::allocate_component<common::PE::CommPattern>("commpattern");
  common::PE::CommPattern& cp = *cp_ptr;
  build_commpattern(cp);
  boost::shared_ptr<LSS::System> sys(common::allocate_component<LSS::System>("sys"));
  sys->options().option("matrix_builder").change_value(matrix_builder);
  build_system(*sys,cp);
  Handle<LSS::Matrix> mat=sys->matrix();
  
  mat->reset(1.);

  
  Handle<LSS::TrilinosVector> sol(sys->solution());
  sol->epetra_vector()->Random();
  //sys->rhs()->reset(1.);
  sol->reset(1.);
  
  mat->apply(sys->rhs(), sys->solution(), 1., 0.);
  
  //sys->solution()->print_native(std::cout);
  sys->rhs()->print(std::cout);
  
//   const Uint nb_blocks = sys->rhs()->blockrow_size();
//   const Uint neq = sys->rhs()->neq();
//   
//   for(Uint i = 0; i != nb_blocks; ++i)
//   {
//     for(Uint j = 0; j != neq; ++j)
//     {
//       if(!cp.isUpdatable()[i])
//         continue;
//       Real result, val;
//       sys->rhs()->get_value(i, j, result);
//       sys->solution()->get_value(i, j, val);
//       BOOST_CHECK_CLOSE(result, 1.+val*4., 1e-12);
//     }
//   }
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

