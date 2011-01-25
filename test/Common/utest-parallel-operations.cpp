// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
//
// IMPORTANT:
// run it both on 1 and many cores
// for example: mpirun -np 4 ./test-parallel-environment --report_level=confirm or --report_level=detailed

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Common 's parallel environment - part of checking operations handling."

////////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/MPI/operations.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace CF;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct PEOperationsFixture
{
  /// common setup for each test case
  PEOperationsFixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~PEOperationsFixture() { }

  /// common params
  int m_argc;
  char** m_argv;

  /// example custom function
  class custommult {
    public:

      /// Static const member variable describing if operation is commutative across processors or not (if not sure, use false).
      static const bool is_commutative=true;

      /// Implementation of the operation. See MPI_Op_create in MPI standard documentation for details.
      template<typename T> static void func(void* in, void* out, int* len, MPI_Datatype* type){
        int rank,i;
        T *in_=(T*)in;
        T *out_=(T*)out;
        for (i=0; i<(const int)(*len); i++) out_[i]*=in_[i];
      }
  };

  /// mimicer function for templatization (basically a substituter for all_reudce)
  template<typename T, typename Op> MPI_Op mimic_usage( T& t, Op ) { return mpi::get_mpi_op<T, Op>::op(); };

};

////////////////////////////////////////////////////////////////////////////////

/// data stays in scope for checking if registration is really static
MPI_Op mpi_op_customplus_i=MPI_OP_NULL;
MPI_Op mpi_op_customplus_d=MPI_OP_NULL;
MPI_Op mpi_op_custommult_i=MPI_OP_NULL;
MPI_Op mpi_op_custommult_d=MPI_OP_NULL;

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( PEOperationsSuite, PEOperationsFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  mpi::PE::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL( mpi::PE::instance().is_init() , true );
  PEProcessSortedExecute(mpi::PE::instance(),-1,CFinfo << "Proccess " << mpi::PE::instance().rank() << "/" << mpi::PE::instance().size() << " reports in." << CFendl;);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( operations_default_types )
{
  int i;
  BOOST_CHECK_EQUAL( mimic_usage(i,mpi::max()),         MPI_MAX);
  BOOST_CHECK_EQUAL( mimic_usage(i,mpi::min()),         MPI_MIN);
  BOOST_CHECK_EQUAL( mimic_usage(i,mpi::plus()),        MPI_SUM);
  BOOST_CHECK_EQUAL( mimic_usage(i,mpi::multiplies()),  MPI_PROD);
  BOOST_CHECK_EQUAL( mimic_usage(i,mpi::logical_and()), MPI_LAND);
  BOOST_CHECK_EQUAL( mimic_usage(i,mpi::logical_or()),  MPI_LOR);
  BOOST_CHECK_EQUAL( mimic_usage(i,mpi::logical_xor()), MPI_LXOR);
  BOOST_CHECK_EQUAL( mimic_usage(i,mpi::bitwise_and()), MPI_BAND);
  BOOST_CHECK_EQUAL( mimic_usage(i,mpi::bitwise_or()),  MPI_BOR);
  BOOST_CHECK_EQUAL( mimic_usage(i,mpi::bitwise_xor()), MPI_BXOR);
  double d;
  BOOST_CHECK_EQUAL( mimic_usage(d,mpi::max()),         MPI_MAX);
  BOOST_CHECK_EQUAL( mimic_usage(d,mpi::min()),         MPI_MIN);
  BOOST_CHECK_EQUAL( mimic_usage(d,mpi::plus()),        MPI_SUM);
  BOOST_CHECK_EQUAL( mimic_usage(d,mpi::multiplies()),  MPI_PROD);
  BOOST_CHECK_EQUAL( mimic_usage(d,mpi::logical_and()), MPI_LAND);
  BOOST_CHECK_EQUAL( mimic_usage(d,mpi::logical_or()),  MPI_LOR);
  BOOST_CHECK_EQUAL( mimic_usage(d,mpi::logical_xor()), MPI_LXOR);
  BOOST_CHECK_EQUAL( mimic_usage(d,mpi::bitwise_and()), MPI_BAND);
  BOOST_CHECK_EQUAL( mimic_usage(d,mpi::bitwise_or()),  MPI_BOR);
  BOOST_CHECK_EQUAL( mimic_usage(d,mpi::bitwise_xor()), MPI_BXOR);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( operations_registered_types )
{
  int i;
  double d;

  // check if registering goes fine
  mpi_op_customplus_i=mimic_usage(i,mpi::customplus());
  BOOST_CHECK_NE(mpi_op_customplus_i,MPI_OP_NULL);
  mpi_op_customplus_d=mimic_usage(d,mpi::customplus());
  BOOST_CHECK_NE(mpi_op_customplus_d,MPI_OP_NULL);
  mpi_op_custommult_i=mimic_usage(i,     custommult());
  BOOST_CHECK_NE(mpi_op_custommult_i,MPI_OP_NULL);
  mpi_op_custommult_d=mimic_usage(d,     custommult());
  BOOST_CHECK_NE(mpi_op_custommult_d,MPI_OP_NULL);

  // check if no glitch and separate types go to separate static variables
  BOOST_CHECK_NE(mpi_op_customplus_i,mpi_op_customplus_d);
  BOOST_CHECK_NE(mpi_op_customplus_i,mpi_op_custommult_i);
  BOOST_CHECK_NE(mpi_op_customplus_i,mpi_op_custommult_d);

  // check if re-registration does not alter the MPI_operations (avoid committing the same type over and over)
  BOOST_CHECK_EQUAL(mpi_op_customplus_i,mimic_usage(i,mpi::customplus()));
  BOOST_CHECK_EQUAL(mpi_op_customplus_d,mimic_usage(d,mpi::customplus()));
  BOOST_CHECK_EQUAL(mpi_op_custommult_i,mimic_usage(i,     custommult()));
  BOOST_CHECK_EQUAL(mpi_op_custommult_d,mimic_usage(d,     custommult()));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( operations_registered_types_are_really_static )
{
  int i;
  double d;

  // check if re-registration does not alter the MPI_operations (avoid committing the same type over and over)
  BOOST_CHECK_EQUAL(mpi_op_customplus_i,mimic_usage(i,mpi::customplus()));
  BOOST_CHECK_EQUAL(mpi_op_customplus_d,mimic_usage(d,mpi::customplus()));
  BOOST_CHECK_EQUAL(mpi_op_custommult_i,mimic_usage(i,     custommult()));
  BOOST_CHECK_EQUAL(mpi_op_custommult_d,mimic_usage(d,     custommult()));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize )
{
  PEProcessSortedExecute(mpi::PE::instance(),-1,CFinfo << "Proccess " << mpi::PE::instance().rank() << "/" << mpi::PE::instance().size() << " says good bye." << CFendl;);
  mpi::PE::instance().finalize();
  BOOST_CHECK_EQUAL( mpi::PE::instance().is_init() , false );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////



