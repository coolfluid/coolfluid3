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
    solvertype("Trilinos"),
    gid(0),
    irank(0),
    nproc(1),
    neq(2),
    rank_updatable(0),
    node_connectivity(0),
    starting_indices(0),
    blockcol_size(0),
    blockrow_size(0)
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
      node_connectivity += 0,2,4,6,1,2,3,5,1,3,5,7,1,2,3,5,0,1,3,4,5,6;
      starting_indices += 0,4,4,8,12,12,16,22,22;
      blockcol_size = 8;
      blockrow_size = 5;
    } else {
      node_connectivity += 1,2,5,0,2,3,4,5,7,1,5,6,2,6,8;
      starting_indices += 0,0,3,9,9,9,12,12,12,15;
      blockcol_size = 9;
      blockrow_size = 4;
    }
    sys->create(cp,neq,node_connectivity,starting_indices);
  }

  /// main solver selector
  std::string solvertype;

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
  sys->options().option("solver").change_value(solvertype);
  build_system(sys,cp);
  LSS::Matrix::Ptr mat=sys->matrix();
  BOOST_CHECK_EQUAL(mat->is_created(),true);
  BOOST_CHECK_EQUAL(mat->solvertype(),solvertype);
  BOOST_CHECK_EQUAL(mat->neq(),neq);
  BOOST_CHECK_EQUAL(mat->blockrow_size(),blockrow_size);
  BOOST_CHECK_EQUAL(mat->blockcol_size(),blockcol_size);

  // just to see if its crashing or not
//  mat->print(std::cout);
//  mat->print(CFinfo);
//  mat->print("test_matrix_" + boost::lexical_cast<std::string>(irank) + ".plt");

  // counter-checking data
  std::vector<Uint> cols(0);
  std::vector<Uint> rows(0);
  std::vector<Real> vals(0);

  // check reset
  mat->reset(1.);
  mat->data(rows,cols,vals);
  BOOST_CHECK_EQUAL(cols.size(),node_connectivity.size()*neq*neq);
  BOOST_CHECK_EQUAL(rows.size(),node_connectivity.size()*neq*neq);
  BOOST_CHECK_EQUAL(vals.size(),node_connectivity.size()*neq*neq);
  BOOST_FOREACH(double v, vals) BOOST_CHECK_EQUAL(v,1.);
  mat->reset();
  mat->data(rows,cols,vals);
  BOOST_FOREACH(double v, vals) BOOST_CHECK_EQUAL(v,0.);

  // diagonal access check
  mat->reset();
  std::vector<Real> diag(blockrow_size*neq,1.);
  mat->set_diagonal(diag);
  mat->add_diagonal(diag);
  diag.clear();
  mat->get_diagonal(diag);
  BOOST_CHECK_EQUAL(diag.size(),blockrow_size*neq);
  BOOST_FOREACH(double i, diag) BOOST_CHECK_EQUAL(i,2.);
  mat->data(rows,cols,vals);
  for (int i=0; i<(const int)vals.size(); i++)
  {
    if (rows[i]==cols[i]) { BOOST_CHECK_EQUAL(vals[i],2.); }
    else { BOOST_CHECK_EQUAL(vals[i],0.); }
  }

  // individual access
  mat->reset();
  if (irank==0)
  {
    mat->set_value(7,6,1.);
    mat->add_value(7,6,1.);
    mat->set_value(6,6,1.);
    mat->set_value(6,7,3.);
    mat->add_value(7,7,4.);
    Real v;
    mat->get_value(6,6,v);
    BOOST_CHECK_EQUAL(v,1.);
    mat->get_value(7,6,v);
    BOOST_CHECK_EQUAL(v,2.);
    mat->get_value(6,7,v);
    BOOST_CHECK_EQUAL(v,3.);
    mat->get_value(7,7,v);
    BOOST_CHECK_EQUAL(v,4.);
    mat->data(rows,cols,vals);
    for (int i=0; i<(const int)vals.size(); i++)
      if ((rows[i]<6)||(rows[i]>7))
        BOOST_CHECK_EQUAL(vals[i],0.);
    for (int i=0; i<(const int)vals.size(); i++)
      if ((cols[i]<6)||(cols[i]>7))
        BOOST_CHECK_EQUAL(vals[i],0.);
  }


  // bc-related functions
  mat->reset(1.);
  mat->get_column_and_replace_to_zero(5,1,vals);
PEDebugVector(vals,vals.size());

  // post-destroy checks
  mat->print("test_matrix_" + boost::lexical_cast<std::string>(irank) + ".plt");


/*
  virtual const bool compatible(const LSS::Vector::Ptr solution, const LSS::Vector::Ptr rhs) = 0;
  virtual void create(CF::Common::Comm::CommPattern& cp, Uint neq, std::vector<Uint>& node_connectivity, std::vector<Uint>& starting_indices, LSS::Vector::Ptr solution, LSS::Vector::Ptr rhs) = 0;
  virtual void destroy() = 0;
  virtual void solve(LSS::Vector::Ptr solution, LSS::Vector::Ptr rhs) = 0;
  virtual void set_values(const BlockAccumulator& values) = 0;
  virtual void add_values(const BlockAccumulator& values) = 0;
  virtual void get_values(BlockAccumulator& values) = 0;
  virtual void set_row(const Uint iblockrow, const Uint ieq, Real diagval, Real offdiagval) = 0;
  virtual void get_column_and_replace_to_zero(const Uint iblockcol, Uint ieq, std::vector<Real>& values) = 0;
  virtual void tie_blockrow_pairs (const Uint iblockrow_to, const Uint iblockrow_from) = 0;

destroy  virtual const bool is_created() = 0;
destroy  virtual const Uint neq() = 0;
destroy  virtual const Uint blockrow_size() = 0;
destroy  virtual const Uint blockcol_size() = 0;
*/

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

