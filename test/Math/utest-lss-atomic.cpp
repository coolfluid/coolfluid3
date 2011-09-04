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
      node_connectivity += 1,2,5,2,5,8,0,7,3,1,2,4,5,6,7,8,2,5,8;
      starting_indices += 0,0,3,9,9,9,16,16,16,19;
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
  std::vector<Real> diag(blockcol_size*neq);
  for (int i=0; i<diag.size(); i++) diag[i]=i;
  mat->set_diagonal(diag);
  mat->add_diagonal(diag);
  diag.clear();
  mat->get_diagonal(diag);
  BOOST_CHECK_EQUAL(diag.size(),blockcol_size*neq);
  mat->data(rows,cols,vals);
  for (int i=0; i<(const int)vals.size(); i++)
  {
    if (cp.isUpdatable()[rows[i]/neq]) { BOOST_CHECK_EQUAL(diag[rows[i]],2.*rows[i]); }
    else { BOOST_CHECK_EQUAL(diag[rows[i]],0.); }
    if (rows[i]==cols[i]) { BOOST_CHECK_EQUAL(vals[i],diag[rows[i]]); }
    else { BOOST_CHECK_EQUAL(vals[i],0.); }
  }

  // individual access
  mat->reset();
  if (irank==0)
  {
    mat->set_value(6,6,1.);
    mat->set_value(7,6,1.);
    mat->add_value(7,6,1.);
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
  if (irank==0)
  {
    mat->get_column_and_replace_to_zero(5,1,vals);
    // check return vector
    // check with non-constant pattern
  }

  mat->print("test_matrix_" + boost::lexical_cast<std::string>(irank) + ".plt");

  mat->reset(1.);
  if (irank==0)
  {
    mat->tie_blockrow_pairs(2,5);
    // check with non-constant pattern
  }

  mat->reset(-1.);
  if (irank==0)
  {
    mat->set_row(3,1,1.,0.);
    mat->data(rows,cols,vals);
    for (int i=0; i<(const int)vals.size(); i++)
    {
      if (rows[i]==7)
      {
        if (cols[i]==7) { BOOST_CHECK_EQUAL(vals[i],1.); }
        else { BOOST_CHECK_EQUAL(vals[i],0.); }
      } else {
        BOOST_CHECK_EQUAL(vals[i],-1.);
      }
    }
  }

  // performant access
  mat->reset();
  if (irank==1)
  {
    LSS::BlockAccumulator ba;
    ba.resize(3,neq);
    ba.mat << 53, 54, 51, 52, 55, 56,
              59, 60, 57, 58, 61, 62,
              23, 24, 21, 22, 25, 26,
              29, 30, 27, 28, 31, 32,
              83, 84, 81, 82, 85, 86,
              89, 90, 87, 88, 91, 92;
    ba.indices[0]=5;
    ba.indices[1]=2;
    ba.indices[2]=8;
    mat->set_values(ba);
    mat->add_values(ba);
    ba.reset();
    ba.indices[0]=2;
    ba.indices[1]=5;
    ba.indices[2]=8;
    mat->get_values(ba);
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(0,i-1),(double)((ba.indices[0]*10+i+0)*2));
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(1,i-1),(double)((ba.indices[0]*10+i+6)*2));
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(2,i-1),(double)((ba.indices[1]*10+i+0)*2));
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(3,i-1),(double)((ba.indices[1]*10+i+6)*2));
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(4,i-1),(double)((ba.indices[2]*10+i+0)*2));
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(5,i-1),(double)((ba.indices[2]*10+i+6)*2));
    mat->data(rows,cols,vals);
    Real ctr=1.;
    for (int i=0; i<(const int)vals.size(); i++)
      if (((rows[i]/neq==ba.indices[0])||(rows[i]/neq==ba.indices[1])||(rows[i]/neq==ba.indices[2]))&&
          ((cols[i]/neq==ba.indices[0])||(cols[i]/neq==ba.indices[1])||(cols[i]/neq==ba.indices[2])))
        {
          BOOST_CHECK_EQUAL(vals[i],(double)((rows[i]/neq*10.+ctr)*2));
          ctr+=1.;
          if (ctr==13.) ctr=1.;
        } else {
          BOOST_CHECK_EQUAL(vals[i],0.);
        }
  }
/*
  // performant access - out of range access does not fail
  mat->reset();
  if (irank==1)
  {
    LSS::BlockAccumulator ba;
    ba.resize(3,neq);
    ba.mat << 53, 54, 51, 52, 55, 56,
              59, 60, 57, 58, 61, 62,
              23, 24, 21, 22, 25, 26,
              29, 30, 27, 28, 31, 32,
              83, 84, 81, 82, 85, 86,
              89, 90, 87, 88, 91, 92;
    ba.indices[0]=4;
    ba.indices[1]=2;
    ba.indices[2]=10;
    mat->set_values(ba);
    mat->add_values(ba);
    ba.reset();
    ba.indices[0]=2;
    ba.indices[1]=4;
    ba.indices[2]=10;
    mat->get_values(ba);
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(0,i-1),(double)((ba.indices[0]*10+i+0)*2));
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(1,i-1),(double)((ba.indices[0]*10+i+6)*2));
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(2,i-1),0.);
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(3,i-1),0.);
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(4,i-1),0.);
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(5,i-1),0.);
    mat->data(rows,cols,vals);
    Real ctr=1.;
    for (int i=0; i<(const int)vals.size(); i++)
      if ((rows[i]/neq==ba.indices[0])&&(cols[i]/neq==ba.indices[0]))
        {
          BOOST_CHECK_EQUAL(vals[i],(double)((rows[i]/neq*10.+ctr)*2));
          ctr+=1.;
          if (ctr==13.) ctr=1.;
        } else {
          BOOST_CHECK_EQUAL(vals[i],0.);
        }
  }
*/
  // post-destroy checks
  mat->destroy();
  BOOST_CHECK_EQUAL(mat->is_created(),false);
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

