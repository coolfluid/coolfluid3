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
  ~LSSAtomicFixture()
  {
  }

  /// create a test commpattern
  void build_commpattern(common::PE::CommPattern& cp)
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
    cp.setup(Handle<common::PE::CommWrapper>(cp.get_child("gid")),rank_updatable);
  }

  /// build a test system
  void build_system(LSS::System& sys, common::PE::CommPattern& cp)
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

BOOST_AUTO_TEST_CASE( test_matrix_only )
{

  // build a commpattern and a matrix
  boost::shared_ptr<common::PE::CommPattern> cp_ptr = common::allocate_component<common::PE::CommPattern>("commpattern");
  common::PE::CommPattern& cp = *cp_ptr;
  build_commpattern(cp);
  boost::shared_ptr<LSS::System> sys(common::allocate_component<LSS::System>("sys"));
  sys->options().option("matrix_builder").change_value(matrix_builder);
  build_system(*sys,cp);
  Handle<LSS::Matrix> mat=sys->matrix();
  BOOST_CHECK_EQUAL(mat->is_created(),true);
  BOOST_CHECK_EQUAL(mat->solvertype(),solvertype);
  BOOST_CHECK_EQUAL(mat->neq(),neq);
  BOOST_CHECK_EQUAL(mat->blockrow_size(),blockrow_size);
  BOOST_CHECK_EQUAL(mat->blockcol_size(),blockcol_size);

  // just to see if its crashing or not
//  mat->print(std::cout);
//  mat->print(CFinfo);
  mat->print("test_matrix_" + boost::lexical_cast<std::string>(irank) + ".plt");

  // counter-checking data
  std::vector<Uint> cols(0);
  std::vector<Uint> rows(0);
  std::vector<Real> vals(0);

  // check reset
  mat->reset(1.);
  mat->debug_data(rows,cols,vals);
  BOOST_CHECK_EQUAL(cols.size(),node_connectivity.size()*neq*neq);
  BOOST_CHECK_EQUAL(rows.size(),node_connectivity.size()*neq*neq);
  BOOST_CHECK_EQUAL(vals.size(),node_connectivity.size()*neq*neq);
  BOOST_FOREACH(double v, vals) BOOST_CHECK_EQUAL(v,1.);
  mat->reset(0.);
  mat->debug_data(rows,cols,vals);
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
  mat->debug_data(rows,cols,vals);
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
    mat->debug_data(rows,cols,vals);
    for (int i=0; i<(const int)vals.size(); i++)
      if ((rows[i]<6)||(rows[i]>7))
        BOOST_CHECK_EQUAL(vals[i],0.);
    for (int i=0; i<(const int)vals.size(); i++)
      if ((cols[i]<6)||(cols[i]>7))
        BOOST_CHECK_EQUAL(vals[i],0.);
  }

  // performant access
  mat->reset();
  if (irank==1)
  {
    LSS::BlockAccumulator ba;
    ba.resize(3,neq);
    ba.mat << 53., 54., 51., 52., 55., 56.,
              59., 60., 57., 58., 61., 62.,
              23., 24., 21., 22., 25., 26.,
              29., 30., 27., 28., 31., 32.,
              83., 84., 81., 82., 85., 86.,
              89., 90., 87., 88., 91., 92.;
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

    mat->print("test_ba_assembly_" + boost::lexical_cast<std::string>(irank) + ".plt");

    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(0,i-1),(double)((ba.indices[0]*10+i+0)*2));
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(1,i-1),(double)((ba.indices[0]*10+i+6)*2));
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(2,i-1),(double)((ba.indices[1]*10+i+0)*2));
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(3,i-1),(double)((ba.indices[1]*10+i+6)*2));
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(4,i-1),(double)((ba.indices[2]*10+i+0)*2));
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(5,i-1),(double)((ba.indices[2]*10+i+6)*2));
    mat->debug_data(rows,cols,vals);
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



  // performant access - out of range access does not fail
  mat->reset();
  if (irank==1)
  {
    LSS::BlockAccumulator ba;
    ba.resize(3,neq);
    ba.mat << 99., 99., 99., 99., 99., 99.,
              99., 99., 99., 99., 99., 99.,
              23., 24., 21., 22., 25., 26.,
              29., 30., 27., 28., 31., 32.,
              99., 99., 99., 99., 99., 99.,
              99., 99., 99., 99., 99., 99.;
    ba.indices[0]=3;
    ba.indices[1]=2;
    ba.indices[2]=7;
    mat->set_values(ba);
    mat->add_values(ba);
    ba.reset(-1.);
    ba.indices[0]=7;
    ba.indices[1]=3;
    ba.indices[2]=2;
    mat->get_values(ba);
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(0,i-1),0.);
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(1,i-1),0.);
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(2,i-1),0.);
    for (int i=1; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(3,i-1),0.);
    for (int i=1; i<3; i++) BOOST_CHECK_EQUAL(ba.mat(4,i-1),(double)((ba.indices[2]*10+i+4+0)*2));
    for (int i=1; i<3; i++) BOOST_CHECK_EQUAL(ba.mat(5,i-1),(double)((ba.indices[2]*10+i+4+6)*2));
    for (int i=3; i<5; i++) BOOST_CHECK_EQUAL(ba.mat(4,i-1),(double)((ba.indices[2]*10+i+0+0)*2));
    for (int i=3; i<5; i++) BOOST_CHECK_EQUAL(ba.mat(5,i-1),(double)((ba.indices[2]*10+i+0+6)*2));
    for (int i=5; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(4,i-1),(double)((ba.indices[2]*10+i-4+0)*2));
    for (int i=5; i<7; i++) BOOST_CHECK_EQUAL(ba.mat(5,i-1),(double)((ba.indices[2]*10+i-4+6)*2));
    mat->debug_data(rows,cols,vals);
    Real ctr=1.;
    for (int i=0; i<(const int)vals.size(); i++)
      if ((rows[i]/neq==ba.indices[2])&&
          ((cols[i]/neq==ba.indices[0])||(cols[i]/neq==ba.indices[1])||(cols[i]/neq==ba.indices[2])))
        {
          BOOST_CHECK_EQUAL(vals[i],(double)((rows[i]/neq*10.+ctr)*2));
          ctr+=1.;
          if (ctr==13.) ctr=1.;
        } else {
          BOOST_CHECK_EQUAL(vals[i],0.);
        }
  }

  // bc-related: dirichlet-condition
  mat->reset(-1.);
  if (irank==0)
  {
    mat->set_row(3,1,1.,0.);
    mat->debug_data(rows,cols,vals);
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

  // bc-related: dirichlet-condition doesnt fail for ghost node
  mat->reset(-1.);
  if (irank==0)
  {
    mat->set_row(1,1,1.,0.);
    mat->debug_data(rows,cols,vals);
    BOOST_FOREACH(Real i, vals) BOOST_CHECK_EQUAL(i,-1.);
  }

  // bc-related: symmetricizing a dirichlet
  try
  {
    mat->reset(1.);
    if (irank==0)
    {
      mat->set_value(11,4,5.);
      mat->set_value(11,5,6.);
      mat->set_value(11,6,7.);
      mat->set_value(11,7,8.);
      mat->set_value(11,10,11.);
      mat->set_value(11,11,12.);
      mat->set_value(11,12,13.);
      mat->set_value(11,13,14.);
      mat->get_column_and_replace_to_zero(5,1,vals);
      vals[0]+=1.;
      vals[1]+=2.;
      vals[2]+=3.;
      vals[3]+=4.;
      vals[8]+=9.;
      vals[9]+=10.;
      vals[14]+=15.;
      vals[15]+=16.;
      for(int i=0; i<vals.size(); i++) BOOST_CHECK_EQUAL(vals[i],(double)(i+1));
      mat->debug_data(rows,cols,vals);
      for (int i=0; i<(const int)vals.size(); i++)
      {
        if (cols[i]==11)
        {
          BOOST_CHECK_EQUAL(vals[i],0.);
        } else {
          BOOST_CHECK_EQUAL(vals[i],1.);
        }
      }
    }
  }
  catch(common::NotImplemented&)
  {
    CFinfo << "skipping symmetric dirichlet test" << CFendl;
  }

  // bc-related: periodicity
  mat->reset(-2.);
  if (irank==0)
  {
    mat->set_value( 2, 4,40.);
    mat->set_value( 3, 4,41.);
    mat->set_value( 4, 4,21.);  // half!
    mat->set_value( 5, 4,21.5); // half!
    mat->set_value( 6, 4,44.);
    mat->set_value( 7, 4,45.);
    mat->set_value(10, 4,23.);  // half!
    mat->set_value(11, 4,23.5); // half!
    mat->set_value( 2, 5,48.);
    mat->set_value( 3, 5,49.);
    mat->set_value( 4, 5,25.);  // half!
    mat->set_value( 5, 5,25.5); // half!
    mat->set_value( 6, 5,52.);
    mat->set_value( 7, 5,53.);
    mat->set_value(10, 5,27.);  // half!
    mat->set_value(11, 5,27.5); // half!

    mat->set_value(11,11, 5.);  // half!
    mat->set_value(10,11, 5.5); // half!
    mat->set_value( 7,11,12.);
    mat->set_value( 6,11,13.);
    mat->set_value( 5,11, 7.);  // half!
    mat->set_value( 4,11, 7.5); // half!
    mat->set_value( 3,11,16.);
    mat->set_value( 2,11,17.);
    mat->set_value(11,10, 9.);  // half!
    mat->set_value(10,10, 9.5); // half!
    mat->set_value( 7,10,20.);
    mat->set_value( 6,10,21.);
    mat->set_value( 5,10,11.);  // half!
    mat->set_value( 4,10,11.5); // half!
    mat->set_value( 3,10,24.);
    mat->set_value( 2,10,25.);

    mat->tie_blockrow_pairs(2,5);

    mat->debug_data(rows,cols,vals);
    for (int i=0; i<(const int)vals.size(); i++)
      if ((rows[i]==4)||(rows[i]==5))
      {
        if ((cols[i]!=10)&&(cols[i]!=11)) { BOOST_CHECK_EQUAL(vals[i],65.); }
        else { BOOST_CHECK_EQUAL(vals[i],0.); }
      }

    for (int i=0; i<(const int)vals.size(); i++)
      if (rows[i]==10)
      {
        if (cols[i]==4) { BOOST_CHECK_EQUAL(vals[i],-1.); }
        else if (cols[i]==10) { BOOST_CHECK_EQUAL(vals[i],1.); }
        else { BOOST_CHECK_EQUAL(vals[i],0.); }
      }

    for (int i=0; i<(const int)vals.size(); i++)
      if (rows[i]==11)
      {
        if (cols[i]==5) { BOOST_CHECK_EQUAL(vals[i],-1.); }
        else if (cols[i]==11) { BOOST_CHECK_EQUAL(vals[i],1.); }
        else { BOOST_CHECK_EQUAL(vals[i],0.); }
      }

    for (int i=0; i<(const int)vals.size(); i++)
      if ((rows[i]!=4)&&(rows[i]!=5)&&(rows[i]!=10)&&(rows[i]!=11))
        BOOST_CHECK_EQUAL(vals[i],-2.);
  }

  // bc-related: periodicity does not fail
  mat->reset(-2.);
  if (irank==0)
  {
    mat->tie_blockrow_pairs(1,4);
    mat->debug_data(rows,cols,vals);
    BOOST_FOREACH(Real i, vals) BOOST_CHECK_EQUAL(i,-2.);
  }

  // post-destroy check
  mat->destroy();
  BOOST_CHECK_EQUAL(mat->is_created(),false);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_vector_only )
{
  // build a commpattern and the two vectors
  boost::shared_ptr<common::PE::CommPattern> cp_ptr = common::allocate_component<common::PE::CommPattern>("commpattern");
  common::PE::CommPattern& cp = *cp_ptr;
  build_commpattern(cp);
  boost::shared_ptr<LSS::System> sys(common::allocate_component<LSS::System>("sys"));
  sys->options().option("matrix_builder").change_value(matrix_builder);
  build_system(*sys,cp);
  Handle<LSS::Vector> sol=sys->solution();
  Handle<LSS::Vector> rhs=sys->rhs();
  BOOST_CHECK_EQUAL(sol->is_created(),true);
  BOOST_CHECK_EQUAL(rhs->is_created(),true);
  BOOST_CHECK_EQUAL(sol->solvertype(),solvertype);
  BOOST_CHECK_EQUAL(rhs->solvertype(),solvertype);
  BOOST_CHECK_EQUAL(sol->neq(),neq);
  BOOST_CHECK_EQUAL(rhs->neq(),neq);
  BOOST_CHECK_EQUAL(sol->blockrow_size(),blockcol_size); // needs to include ghost entries
  BOOST_CHECK_EQUAL(rhs->blockrow_size(),blockcol_size); // needs to include ghost entries

  // just to see if its crashing or not
//  sol->print(std::cout);
//  sol->print(CFinfo);
  sol->print("test_vector_" + boost::lexical_cast<std::string>(irank) + ".plt");

  // counter-checking data
  std::vector<Real> vals(0);
  Real val;

  // check reset
  sol->reset(1.);
  sol->debug_data(vals);
  BOOST_CHECK_EQUAL(vals.size(),gid.size()*neq);
  BOOST_FOREACH(double v, vals) BOOST_CHECK_EQUAL(v,1.);
  sol->reset();
  sol->debug_data(vals);
  BOOST_FOREACH(double v, vals) BOOST_CHECK_EQUAL(v,0.);

  // set by row-wise index
  sol->reset();
  sol->set_value(5,1.);
  sol->add_value(5,1.);
  val=0.;
  sol->get_value(5,val);
  BOOST_CHECK_EQUAL(val,2.);
  sol->debug_data(vals);
  for (int i=0; i<vals.size(); i++)
  {
    if (i==5) { BOOST_CHECK_EQUAL(vals[i],2.); }
    else { BOOST_CHECK_EQUAL(vals[i],0.); }
  }

  // set by block-wise index
  sol->reset();
  sol->set_value(2,1,1.);
  sol->add_value(2,1,1.);
  val=0.;
  sol->get_value(2,1,val);
  BOOST_CHECK_EQUAL(val,2.);
  sol->debug_data(vals);
  for (int i=0; i<vals.size(); i++)
  {
    if (i==5) { BOOST_CHECK_EQUAL(vals[i],2.); }
    else { BOOST_CHECK_EQUAL(vals[i],0.); }
  }

  // set via blockaccumulator
  sol->reset();
  BlockAccumulator ba;
  ba.resize(3,neq);
  ba.rhs << 8.,9.   ,  2.,3.   ,  6.,7.;
  ba.indices[0]=4;
  ba.indices[1]=1;
  ba.indices[2]=3;
  sol->set_rhs_values(ba);
  sol->add_rhs_values(ba);
  ba.reset();
  sol->get_rhs_values(ba);
  sol->debug_data(vals);
  for (int i=0; i<vals.size(); i++)
  {
    if ((i/neq==ba.indices[0])||(i/neq==ba.indices[1])||(i/neq==ba.indices[2]))
    {
      BOOST_CHECK_EQUAL(vals[i],(double)(i*2));
    } else {
      BOOST_CHECK_EQUAL(vals[i],0.);
    }
  }
  BOOST_CHECK_EQUAL(ba.rhs[0],16.);
  BOOST_CHECK_EQUAL(ba.rhs[1],18.);
  BOOST_CHECK_EQUAL(ba.rhs[2],4.);
  BOOST_CHECK_EQUAL(ba.rhs[3],6.);
  BOOST_CHECK_EQUAL(ba.rhs[4],12.);
  BOOST_CHECK_EQUAL(ba.rhs[5],14.);
  BOOST_CHECK_EQUAL(ba.sol.isConstant(0.,1.e-10),true);

  sol->reset();
  ba.sol << 80.,90. ,  20.,30. ,  60.,70.;
  sol->set_sol_values(ba);
  sol->add_sol_values(ba);
  ba.reset();
  sol->get_sol_values(ba);
  sol->debug_data(vals);
  for (int i=0; i<vals.size(); i++)
  {
    if ((i/neq==ba.indices[0])||(i/neq==ba.indices[1])||(i/neq==ba.indices[2]))
    {
      BOOST_CHECK_EQUAL(vals[i],(double)(i*20.));
    } else {
      BOOST_CHECK_EQUAL(vals[i],0.);
    }
  }
  BOOST_CHECK_EQUAL(ba.sol[0],160.);
  BOOST_CHECK_EQUAL(ba.sol[1],180.);
  BOOST_CHECK_EQUAL(ba.sol[2],40.);
  BOOST_CHECK_EQUAL(ba.sol[3],60.);
  BOOST_CHECK_EQUAL(ba.sol[4],120.);
  BOOST_CHECK_EQUAL(ba.sol[5],140.);
  BOOST_CHECK_EQUAL(ba.rhs.isConstant(0.,1.e-10),true);

  // check destroy
  sol->destroy();
  BOOST_CHECK_EQUAL(sol->is_created(),false);
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

  // just to see if its crashing or not
//  sys->print(std::cout);
//  sys->print(CFinfo);
  sys->print("test_system_" + boost::lexical_cast<std::string>(irank) + ".plt");

  // counter-checking data
  std::vector<Uint> cols(0);
  std::vector<Uint> rows(0);
  std::vector<Real> vals(0);

  // reset
  sys->reset(1.);
  sys->matrix()->debug_data(rows,cols,vals);
  BOOST_CHECK_EQUAL(cols.size(),node_connectivity.size()*neq*neq);
  BOOST_CHECK_EQUAL(rows.size(),node_connectivity.size()*neq*neq);
  BOOST_CHECK_EQUAL(vals.size(),node_connectivity.size()*neq*neq);
  BOOST_FOREACH(Real i,vals) BOOST_CHECK_EQUAL(i,1.);
  sys->solution()->debug_data(vals);
  BOOST_CHECK_EQUAL(vals.size(),gid.size()*neq);
  BOOST_FOREACH(Real i,vals) BOOST_CHECK_EQUAL(i,1.);
  sys->rhs()->debug_data(vals);
  BOOST_CHECK_EQUAL(vals.size(),gid.size()*neq);
  BOOST_FOREACH(Real i,vals) BOOST_CHECK_EQUAL(i,1.);
  sys->reset();
  sys->matrix()->debug_data(rows,cols,vals);
  BOOST_CHECK_EQUAL(cols.size(),node_connectivity.size()*neq*neq);
  BOOST_CHECK_EQUAL(rows.size(),node_connectivity.size()*neq*neq);
  BOOST_CHECK_EQUAL(vals.size(),node_connectivity.size()*neq*neq);
  BOOST_FOREACH(Real i,vals) BOOST_CHECK_EQUAL(i,0.);
  sys->solution()->debug_data(vals);
  BOOST_CHECK_EQUAL(vals.size(),gid.size()*neq);
  BOOST_FOREACH(Real i,vals) BOOST_CHECK_EQUAL(i,0.);
  sys->rhs()->debug_data(vals);
  BOOST_CHECK_EQUAL(vals.size(),gid.size()*neq);
  BOOST_FOREACH(Real i,vals) BOOST_CHECK_EQUAL(i,0.);

  // dirichlet bc
  sys->matrix()->reset(2.);
  sys->solution()->reset(3.);
  sys->rhs()->reset(4.);
  if (irank==0)
  {
    sys->dirichlet(3,1,5.,false);
    sys->matrix()->debug_data(rows,cols,vals);
    for (int i=0; i<vals.size(); i++)
    {
      if (rows[i]==7)
      {
        if (cols[i]==7) { BOOST_CHECK_EQUAL(vals[i],1.); }
        else { BOOST_CHECK_EQUAL(vals[i],0.); }
      } else {
        if (cols[i]==7)
        {
          //BOOST_CHECK_EQUAL(vals[i],0.); // only works for symmetric Dirichlet BC
        }
        else
        {
          BOOST_CHECK_EQUAL(vals[i],2.);
        }
      }
    }
    sys->solution()->debug_data(vals);
    for (int i=0; i<vals.size(); i++)
    {
      if (i==7) { BOOST_CHECK_EQUAL(vals[i],5.); }
      else { BOOST_CHECK_EQUAL(vals[i],3.); }
    }
    sys->rhs()->debug_data(vals);
    for (int i=0; i<4; i++) BOOST_CHECK_EQUAL(vals[i],4.);
    for (int i=4; i<7; i++) BOOST_CHECK_EQUAL(vals[i],4.);
    for (int i=7; i<8; i++) BOOST_CHECK_EQUAL(vals[i],5.);
    for (int i=8; i<10; i++) BOOST_CHECK_EQUAL(vals[i],4.);
    for (int i=10; i<14; i++) BOOST_CHECK_EQUAL(vals[i],4.);
    for (int i=14; i<16; i++) BOOST_CHECK_EQUAL(vals[i],4.);
  }

  // performant access - out of range access does not fail
  sys->reset();
  if (irank==1)
  {
    LSS::BlockAccumulator ba;
    ba.resize(3,neq);
    ba.mat << 99., 99., 99., 99., 99., 99.,
              99., 99., 99., 99., 99., 99.,
              1.,   1.,  1.,  1.,  1.,  1.,
              1.,   1.,  1.,  1.,  1.,  1.,
              99., 99., 99., 99., 99., 99.,
              99., 99., 99., 99., 99., 99.;
    ba.rhs << 2., 2., 2., 2., 2., 2.;
    ba.sol << 3., 3., 3., 3., 3., 3.;
    ba.indices[0]=3;
    ba.indices[1]=2;
    ba.indices[2]=7;
    sys->set_values(ba);
    sys->add_values(ba);
    ba.reset();
    ba.indices[0]=7;
    ba.indices[1]=3;
    ba.indices[2]=2;
    sys->get_values(ba);
    sys->matrix()->debug_data(rows,cols,vals);
    for (int i=0; i<(const int)vals.size(); i++)
    {
      if ((rows[i]/neq==ba.indices[2])&&
          ((cols[i]/neq==ba.indices[0])||(cols[i]/neq==ba.indices[1])||(cols[i]/neq==ba.indices[2]))) { BOOST_CHECK_EQUAL(vals[i],2.); }
      else { BOOST_CHECK_EQUAL(vals[i],0.); }
    }
    sys->rhs()->debug_data(vals);
    for (int i=0; i<(const int)vals.size(); i++)
    {
      if ((i/neq==ba.indices[0])||(i/neq==ba.indices[1])||(i/neq==ba.indices[2])) { BOOST_CHECK_EQUAL(vals[i],4.); }
      else { BOOST_CHECK_EQUAL(vals[i],0.); }
    }
    sys->solution()->debug_data(vals);
    for (int i=0; i<(const int)vals.size(); i++)
    {
      if ((i/neq==ba.indices[0])||(i/neq==ba.indices[1])||(i/neq==ba.indices[2])) { BOOST_CHECK_EQUAL(vals[i],6.); }
      else { BOOST_CHECK_EQUAL(vals[i],0.); }
    }
  }

  // check periodicity
  sys->matrix()->reset(-2.);
  sys->solution()->reset(-3.);
  sys->rhs()->reset(-4.);
  if (irank==0)
  {
    sys->matrix()->set_value( 2, 4,40.);
    sys->matrix()->set_value( 3, 4,41.);
    sys->matrix()->set_value( 4, 4,21.);  // half!
    sys->matrix()->set_value( 5, 4,21.5); // half!
    sys->matrix()->set_value( 6, 4,44.);
    sys->matrix()->set_value( 7, 4,45.);
    sys->matrix()->set_value(10, 4,23.);  // half!
    sys->matrix()->set_value(11, 4,23.5); // half!
    sys->matrix()->set_value( 2, 5,48.);
    sys->matrix()->set_value( 3, 5,49.);
    sys->matrix()->set_value( 4, 5,25.);  // half!
    sys->matrix()->set_value( 5, 5,25.5); // half!
    sys->matrix()->set_value( 6, 5,52.);
    sys->matrix()->set_value( 7, 5,53.);
    sys->matrix()->set_value(10, 5,27.);  // half!
    sys->matrix()->set_value(11, 5,27.5); // half!

    sys->matrix()->set_value(11,11, 5.);  // half!
    sys->matrix()->set_value(10,11, 5.5); // half!
    sys->matrix()->set_value( 7,11,12.);
    sys->matrix()->set_value( 6,11,13.);
    sys->matrix()->set_value( 5,11, 7.);  // half!
    sys->matrix()->set_value( 4,11, 7.5); // half!
    sys->matrix()->set_value( 3,11,16.);
    sys->matrix()->set_value( 2,11,17.);
    sys->matrix()->set_value(11,10, 9.);  // half!
    sys->matrix()->set_value(10,10, 9.5); // half!
    sys->matrix()->set_value( 7,10,20.);
    sys->matrix()->set_value( 6,10,21.);
    sys->matrix()->set_value( 5,10,11.);  // half!
    sys->matrix()->set_value( 4,10,11.5); // half!
    sys->matrix()->set_value( 3,10,24.);
    sys->matrix()->set_value( 2,10,25.);

    sys->solution()->set_value(4,-1.);
    sys->solution()->set_value(5, 0.);
    sys->solution()->set_value(10,-5.);
    sys->solution()->set_value(11,-6.);

    sys->rhs()->set_value(4,3.);
    sys->rhs()->set_value(5,4.);
    sys->rhs()->set_value(10,-7.);
    sys->rhs()->set_value(11,-8.);

    sys->periodicity(2,5);

    sys->matrix()->debug_data(rows,cols,vals);
    for (int i=0; i<(const int)vals.size(); i++)
      if ((rows[i]==4)||(rows[i]==5))
      {
        if ((cols[i]!=10)&&(cols[i]!=11)) { BOOST_CHECK_EQUAL(vals[i],65.); }
        else { BOOST_CHECK_EQUAL(vals[i],0.); }
      }

    for (int i=0; i<(const int)vals.size(); i++)
      if (rows[i]==10)
      {
        if (cols[i]==4) { BOOST_CHECK_EQUAL(vals[i],-1.); }
        else if (cols[i]==10) { BOOST_CHECK_EQUAL(vals[i],1.); }
        else { BOOST_CHECK_EQUAL(vals[i],0.); }
      }

    for (int i=0; i<(const int)vals.size(); i++)
      if (rows[i]==11)
      {
        if (cols[i]==5) { BOOST_CHECK_EQUAL(vals[i],-1.); }
        else if (cols[i]==11) { BOOST_CHECK_EQUAL(vals[i],1.); }
        else { BOOST_CHECK_EQUAL(vals[i],0.); }
      }

    for (int i=0; i<(const int)vals.size(); i++)
      if ((rows[i]!=4)&&(rows[i]!=5)&&(rows[i]!=10)&&(rows[i]!=11))
        BOOST_CHECK_EQUAL(vals[i],-2.);

    sys->solution()->debug_data(vals);
    BOOST_FOREACH(Real i, vals) BOOST_CHECK_EQUAL(i,-3.);

    sys->rhs()->debug_data(vals);
    for (int i=0; i<vals.size(); i++)
    {
      if ((i==10)||(i==11)) { BOOST_CHECK_EQUAL(vals[i],0.); }
      else { BOOST_CHECK_EQUAL(vals[i],-4.); }
    }
  }

  // diagonal access check
  sys->reset();
  std::vector<Real> diag(blockcol_size*neq);
  for (int i=0; i<diag.size(); i++) diag[i]=i;
  sys->set_diagonal(diag);
  sys->add_diagonal(diag);
  diag.clear();
  sys->get_diagonal(diag);
  BOOST_CHECK_EQUAL(diag.size(),blockcol_size*neq);
  sys->matrix()->debug_data(rows,cols,vals);
  for (int i=0; i<(const int)vals.size(); i++)
  {
    if (cp.isUpdatable()[rows[i]/neq]) { BOOST_CHECK_EQUAL(diag[rows[i]],2.*rows[i]); }
    else { BOOST_CHECK_EQUAL(diag[rows[i]],0.); }
    if (rows[i]==cols[i]) { BOOST_CHECK_EQUAL(vals[i],diag[rows[i]]); }
    else { BOOST_CHECK_EQUAL(vals[i],0.); }
  }

  // test swapping rhs and sol
  boost::shared_ptr<LSS::System> sys2(common::allocate_component<LSS::System>("sys2"));
  sys->options().option("matrix_builder").change_value(matrix_builder);
  build_system(*sys2,cp);
  BOOST_CHECK_EQUAL(sys2->is_created(),true);
  BOOST_CHECK_EQUAL(sys2->solvertype(),solvertype);
  sys->reset(1.);
  sys2->reset(2.);
  boost::shared_ptr<Matrix> sw_mat = boost::dynamic_pointer_cast<Matrix>(sys->remove_component("Matrix"));
  boost::shared_ptr<Vector> sw_sol = boost::dynamic_pointer_cast<Vector>(sys2->remove_component("Solution"));
  boost::shared_ptr<Vector> sw_rhs = boost::dynamic_pointer_cast<Vector>(sys2->remove_component("RHS"));
  sys->swap(sw_mat,sw_sol,sw_rhs);
  sys->matrix()->debug_data(rows,cols,vals);
  BOOST_FOREACH(Real i, vals) BOOST_CHECK_EQUAL(i,1.);
  sys->solution()->debug_data(vals);
  BOOST_FOREACH(Real i, vals) BOOST_CHECK_EQUAL(i,2.);
  sys->rhs()->debug_data(vals);
  BOOST_FOREACH(Real i, vals) BOOST_CHECK_EQUAL(i,2.);

  // check destroy
  sys->destroy();
  BOOST_CHECK_EQUAL(sys->is_created(),false);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( solve_system )
{

// THE SERIAL IS EQUIVALENT WITH THE FOLLOWING OCTAVE/MATLAB CODE
// A =  [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; -0.5, -0.5, 1, -0.5, -0.5, -0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; -0.5, -0.5, -0.5, 1, -0.5, -0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, -0.5, -0.5, 1, -0.5, -0.5, -0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, -0.5, -0.5, -0.5, 1, -0.5, -0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, -0.5, -0.5, 1, -0.5, -0.5, -0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, -0.5, -0.5, -0.5, 1, -0.5, -0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, -0.5, -0.5, 1, -0.5, -0.5, -0.5, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, -0.5, -0.5, -0.5, 1, -0.5, -0.5, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, -0.5, -0.5, 1, -0.5, -0.5, -0.5, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, -0.5, -0.5, -0.5, 1, -0.5, -0.5, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -0.5, -0.5, 1, -0.5, -0.5, -0.5, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -0.5, -0.5, -0.5, 1, -0.5, -0.5, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -0.5, -0.5, 1, -0.5, -0.5, -0.5, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -0.5, -0.5, -0.5, 1, -0.5, -0.5, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -0.5, -0.5, 1, -0.5, -0.5, -0.5; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -0.5, -0.5, -0.5, 1, -0.5, -0.5; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1]
// b =  [1; 1; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 10; 10]
// inv(A)*b
// WHICH RESULTS IN GID ORDER:

  std::vector<Real> refvals(0);
  refvals +=
   1.00000000000000e+00,
   1.00000000000000e+00,
  -1.35789473684210e+01,
  -1.35789473684210e+01,
  -7.78947368421052e+00,
  -7.78947368421052e+00,
   9.68421052631579e+00,
   9.68421052631579e+00,
   1.26315789473684e+01,
   1.26315789473684e+01,
  -3.36842105263158e+00,
  -3.36842105263158e+00,
  -1.43157894736842e+01,
  -1.43157894736842e+01,
  -3.78947368421053e+00,
  -3.78947368421052e+00,
   1.24210526315789e+01,
   1.24210526315789e+01,
   1.00000000000000e+01,
   1.00000000000000e+01;

  // commpattern
  if (irank==0)
  {
    gid += 0,1,2,3,4;
    rank_updatable += 0,0,0,0,1;
  } else {
    gid += 3,4,5,6,7,8,9;
    rank_updatable += 0,1,1,1,1,1,1;
  }
  boost::shared_ptr<common::PE::CommPattern> cp_ptr = common::allocate_component<common::PE::CommPattern>("commpattern");
  common::PE::CommPattern& cp = *cp_ptr;
  cp.insert("gid",gid,1,false);
  cp.setup(Handle<common::PE::CommWrapper>(cp.get_child("gid")),rank_updatable);

  // lss
  if (irank==0)
  {
    node_connectivity += 0,1,0,1,2,1,2,3,2,3,4,3,4;
    starting_indices += 0,2,5,8,11,13;
  } else {
    node_connectivity += 0,1,0,1,2,1,2,3,2,3,4,3,4,5,4,5,6,5,6;
    starting_indices +=  0,2,5,8,11,14,17,19;
  }
  boost::shared_ptr<System> sys(common::allocate_component<System>("sys"));
  sys->options().option("matrix_builder").change_value(matrix_builder);
  sys->create(cp,2,node_connectivity,starting_indices);

  sys->solution_strategy()->options().set("compute_residual", true);
  sys->solution_strategy()->options().set("verbosity_level", 3);
  sys->solution_strategy()->access_component("Parameters")->options().set("preconditioner_type", std::string("None"));
  sys->solution_strategy()->access_component("Parameters/LinearSolverTypes/Belos/SolverTypes/BlockGMRES")->options().set("verbosity", 1);

  // set intital values and boundary conditions
  sys->matrix()->reset(-0.5);
  sys->solution()->reset(1.);
  sys->rhs()->reset(0.);
  if (irank==0)
  {
    std::vector<Real> diag(10,1.);
    sys->set_diagonal(diag);
    sys->dirichlet(0,0,1.);
    sys->dirichlet(0,1,1.);
  } else {
    std::vector<Real> diag(14,1.);
    sys->set_diagonal(diag);
    sys->dirichlet(6,0,10.);
    sys->dirichlet(6,1,10.);
  }

  // solve and check
  sys->solve();
  std::vector<Real> vals;
  sys->solution()->debug_data(vals);
  for (int i=0; i<vals.size(); i++)
    if (cp.isUpdatable()[i/neq])
      BOOST_CHECK_CLOSE( vals[i], refvals[gid[i/neq]*neq], 1e-8);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( solve_system_blocked )
{

// THE SERIAL IS EQUIVALENT WITH THE FOLLOWING OCTAVE/MATLAB CODE
// A =  [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; -0.5, -0.5, 1, -0.5, -0.5, -0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; -0.5, -0.5, -0.5, 1, -0.5, -0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, -0.5, -0.5, 1, -0.5, -0.5, -0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, -0.5, -0.5, -0.5, 1, -0.5, -0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, -0.5, -0.5, 1, -0.5, -0.5, -0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, -0.5, -0.5, -0.5, 1, -0.5, -0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, -0.5, -0.5, 1, -0.5, -0.5, -0.5, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, -0.5, -0.5, -0.5, 1, -0.5, -0.5, 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, -0.5, -0.5, 1, -0.5, -0.5, -0.5, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, -0.5, -0.5, -0.5, 1, -0.5, -0.5, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -0.5, -0.5, 1, -0.5, -0.5, -0.5, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -0.5, -0.5, -0.5, 1, -0.5, -0.5, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -0.5, -0.5, 1, -0.5, -0.5, -0.5, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -0.5, -0.5, -0.5, 1, -0.5, -0.5, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -0.5, -0.5, 1, -0.5, -0.5, -0.5; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -0.5, -0.5, -0.5, 1, -0.5, -0.5; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0; 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1]
// b =  [1; 1; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 10; 10]
// inv(A)*b
// WHICH RESULTS IN GID ORDER:

  try
  {
  std::vector<Real> refvals(0);
  refvals +=
   1.00000000000000e+00,
   1.00000000000000e+00,
  -1.35789473684210e+01,
  -1.35789473684210e+01,
  -7.78947368421052e+00,
  -7.78947368421052e+00,
   9.68421052631579e+00,
   9.68421052631579e+00,
   1.26315789473684e+01,
   1.26315789473684e+01,
  -3.36842105263158e+00,
  -3.36842105263158e+00,
  -1.43157894736842e+01,
  -1.43157894736842e+01,
  -3.78947368421053e+00,
  -3.78947368421052e+00,
   1.24210526315789e+01,
   1.24210526315789e+01,
   1.00000000000000e+01,
   1.00000000000000e+01;

  // commpattern
  if (irank==0)
  {
    gid += 0,1,2,3,4;
    rank_updatable += 0,0,0,0,1;
  } else {
    gid += 3,4,5,6,7,8,9;
    rank_updatable += 0,1,1,1,1,1,1;
  }
  boost::shared_ptr<common::PE::CommPattern> cp_ptr = common::allocate_component<common::PE::CommPattern>("commpattern");
  common::PE::CommPattern& cp = *cp_ptr;
  cp.insert("gid",gid,1,false);
  cp.setup(Handle<common::PE::CommWrapper>(cp.get_child("gid")),rank_updatable);

  // lss
  if (irank==0)
  {
    node_connectivity += 0,1,0,1,2,1,2,3,2,3,4,3,4;
    starting_indices += 0,2,5,8,11,13;
  } else {
    node_connectivity += 0,1,0,1,2,1,2,3,2,3,4,3,4,5,4,5,6,5,6;
    starting_indices +=  0,2,5,8,11,14,17,19;
  }
  boost::shared_ptr<System> sys(common::allocate_component<System>("sys"));
  sys->options().option("matrix_builder").change_value(matrix_builder);
  boost::shared_ptr<math::VariablesDescriptor> vars = common::allocate_component<math::VariablesDescriptor>("vars");
  vars->options().set("dimension", 1u);

  vars->push_back("var1", cf3::math::VariablesDescriptor::Dimensionalities::SCALAR);
  vars->push_back("var2", cf3::math::VariablesDescriptor::Dimensionalities::SCALAR);
  sys->create_blocked(cp,*vars,node_connectivity,starting_indices);

  sys->solution_strategy()->options().set("compute_residual", true);
  sys->solution_strategy()->options().set("verbosity_level", 3);
  sys->solution_strategy()->access_component("Parameters")->options().set("preconditioner_type", std::string("None"));
  sys->solution_strategy()->access_component("Parameters/LinearSolverTypes/Belos/SolverTypes/BlockGMRES")->options().set("verbosity", 1);
  sys->solution_strategy()->access_component("Parameters/LinearSolverTypes/Belos/SolverTypes/BlockGMRES")->options().set("convergence_tolerance", 0.1);

  // set intital values and boundary conditions
  sys->matrix()->reset(-0.5);
  sys->solution()->reset(1.);
  sys->rhs()->reset(0.);
  if (irank==0)
  {
    std::vector<Real> diag(10,1.);
    sys->set_diagonal(diag);
    sys->dirichlet(0,0,1.);
    sys->dirichlet(0,1,1.);
  } else {
    std::vector<Real> diag(14,1.);
    sys->set_diagonal(diag);
    sys->dirichlet(6,0,10.);
    sys->dirichlet(6,1,10.);
  }

  // solve and check
  sys->solve();
  std::vector<Real> vals;
  sys->solution()->debug_data(vals);
  for (int i=0; i<vals.size(); i++)
    if (cp.isUpdatable()[i/neq])
      BOOST_CHECK_CLOSE( vals[i], refvals[gid[i/neq]*neq], 1e-8);
  }
  catch(common::NotImplemented&)
  {
    CFinfo << "skipping blocked solve" << CFendl;
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

