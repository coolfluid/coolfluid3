// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
//
// IMPORTANT:
// run it both on 1 and many cores
// for example: mpirun -np 4 ./test-parallel-environment --report_level=confirm or --report_level=detailed

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::common 's parallel environment - part of checking operations handling."

////////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/operations.hpp"

#include "common/PE/debug.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::common;

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

      /// Implementation of the operation. See Operation_create in MPI standard documentation for details.
      template<typename T> static void func(void* in, void* out, int* len, PE::Datatype* type){
        int rank,i;
        T *in_=(T*)in;
        T *out_=(T*)out;
        for (i=0; i<*len; i++) out_[i]= out_[i]*in_[i];
      }
  };

  /// mimicer function for templatization (basically a substituter for all_reudce)
  template<typename T, typename Op> PE::Operation mimic_usage( T& t, Op ) { return PE::get_mpi_op<T, Op>::op(); };

  /// custom class for checking the non built-in way
  class optest {
    public:
      int ival;
      double dval;
      // note that these operators are bogus
      optest operator + (const optest& b) const { optest t; t.ival=-1; t.dval=-1.; return t; };
      optest operator * (const optest& b) const { optest t; t.ival=-1; t.dval=-1.; return t; };
      bool   operator > (const optest& b) const { return true; };
      bool   operator < (const optest& b) const { return true; };
      optest operator &&(const optest& b) const { optest t; t.ival=-1; t.dval=-1.; return t; };
      optest operator ||(const optest& b) const { optest t; t.ival=-1; t.dval=-1.; return t; };
      optest operator ! ()                const { optest t; t.ival=-1; t.dval=-1.; return t; };
      optest operator & (const optest& b) const { optest t; t.ival=-1; t.dval=-1.; return t; };
      optest operator | (const optest& b) const { optest t; t.ival=-1; t.dval=-1.; return t; };
      optest operator ^ (const optest& b) const { optest t; t.ival=-1; t.dval=-1.; return t; };
  };

  /// helper function for testing all operations on a type
  template <typename T> void test_all_operations(){
    T t;
    BOOST_CHECK_EQUAL( mimic_usage(t,PE::max()),         MPI_MAX);
    BOOST_CHECK_EQUAL( mimic_usage(t,PE::min()),         MPI_MIN);
    BOOST_CHECK_EQUAL( mimic_usage(t,PE::plus()),        MPI_SUM);
    BOOST_CHECK_EQUAL( mimic_usage(t,PE::multiplies()),  MPI_PROD);
    BOOST_CHECK_EQUAL( mimic_usage(t,PE::logical_and()), MPI_LAND);
    BOOST_CHECK_EQUAL( mimic_usage(t,PE::logical_or()),  MPI_LOR);
    BOOST_CHECK_EQUAL( mimic_usage(t,PE::logical_xor()), MPI_LXOR);
    BOOST_CHECK_EQUAL( mimic_usage(t,PE::bitwise_and()), MPI_BAND);
    BOOST_CHECK_EQUAL( mimic_usage(t,PE::bitwise_or()),  MPI_BOR);
    BOOST_CHECK_EQUAL( mimic_usage(t,PE::bitwise_xor()), MPI_BXOR);
  }

};

////////////////////////////////////////////////////////////////////////////////

/// data stays in scope for checking if registration is really static
PE::Operation mpi_op_customplus_i=(PE::Operation)nullptr;
PE::Operation mpi_op_customplus_d=(PE::Operation)nullptr;
PE::Operation mpi_op_customplus_o=(PE::Operation)nullptr;
PE::Operation mpi_op_custommult_i=(PE::Operation)nullptr;
PE::Operation mpi_op_custommult_d=(PE::Operation)nullptr;
PE::Operation mpi_op_custommult_o=(PE::Operation)nullptr;

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( PEOperationsSuite, PEOperationsFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  PE::Comm::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL( PE::Comm::instance().is_active() , true );
  PEProcessSortedExecute(-1,CFinfo << "Proccess " << PE::Comm::instance().rank() << "/" << PE::Comm::instance().size() << " reports in." << CFendl;);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( operations_built_in_types )
{
  // each called two times to check re-registration
  test_all_operations<char>();
  test_all_operations<char>();
  test_all_operations<unsigned char>();
  test_all_operations<unsigned char>();
  test_all_operations<short>();
  test_all_operations<short>();
  test_all_operations<unsigned short>();
  test_all_operations<unsigned short>();
  test_all_operations<int>();
  test_all_operations<int>();
  test_all_operations<unsigned int>();
  test_all_operations<unsigned int>();
  test_all_operations<long>();
  test_all_operations<long>();
  test_all_operations<unsigned long>();
  test_all_operations<unsigned long>();
  test_all_operations<long long>();
  test_all_operations<long long>();
  test_all_operations<unsigned long long>();
  test_all_operations<unsigned long long>();
  test_all_operations<float>();
  test_all_operations<float>();
  test_all_operations<double>();
  test_all_operations<double>();
  test_all_operations<long double>();
  test_all_operations<long double>();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( operations_registered_types )
{
  int i;
  double d;
  optest o;

  // check if registering goes fine
  mpi_op_customplus_i=mimic_usage(i,PE::customplus());
  BOOST_CHECK_NE(mpi_op_customplus_i,(PE::Operation)nullptr);
  mpi_op_customplus_d=mimic_usage(d,PE::customplus());
  BOOST_CHECK_NE(mpi_op_customplus_d,(PE::Operation)nullptr);
  mpi_op_customplus_o=mimic_usage(o,PE::customplus());
  BOOST_CHECK_NE(mpi_op_customplus_o,(PE::Operation)nullptr);
  mpi_op_custommult_i=mimic_usage(i,     custommult());
  BOOST_CHECK_NE(mpi_op_custommult_i,(PE::Operation)nullptr);
  mpi_op_custommult_d=mimic_usage(d,     custommult());
  BOOST_CHECK_NE(mpi_op_custommult_d,(PE::Operation)nullptr);
  mpi_op_custommult_o=mimic_usage(o,     custommult());
  BOOST_CHECK_NE(mpi_op_custommult_o,(PE::Operation)nullptr);

  // check if no glitch and separate types go to separate static variables
  BOOST_CHECK_NE(mpi_op_customplus_i,mpi_op_customplus_d);
  BOOST_CHECK_NE(mpi_op_customplus_i,mpi_op_customplus_o);
  BOOST_CHECK_NE(mpi_op_custommult_i,mpi_op_custommult_d);
  BOOST_CHECK_NE(mpi_op_custommult_i,mpi_op_custommult_o);
  BOOST_CHECK_NE(mpi_op_customplus_i,mpi_op_custommult_i);
  BOOST_CHECK_NE(mpi_op_customplus_i,mpi_op_custommult_d);
  BOOST_CHECK_NE(mpi_op_customplus_i,mpi_op_custommult_o);
  BOOST_CHECK_NE(mpi_op_customplus_d,mpi_op_custommult_i);
  BOOST_CHECK_NE(mpi_op_customplus_d,mpi_op_custommult_d);
  BOOST_CHECK_NE(mpi_op_customplus_d,mpi_op_custommult_o);
  BOOST_CHECK_NE(mpi_op_customplus_o,mpi_op_custommult_i);
  BOOST_CHECK_NE(mpi_op_customplus_o,mpi_op_custommult_d);
  BOOST_CHECK_NE(mpi_op_customplus_o,mpi_op_custommult_o);

  // check if re-registration does not alter the MPI_operations (avoid committing the same type over and over)
  BOOST_CHECK_EQUAL(mpi_op_customplus_i,mimic_usage(i,PE::customplus()));
  BOOST_CHECK_EQUAL(mpi_op_customplus_d,mimic_usage(d,PE::customplus()));
  BOOST_CHECK_EQUAL(mpi_op_customplus_o,mimic_usage(o,PE::customplus()));
  BOOST_CHECK_EQUAL(mpi_op_custommult_i,mimic_usage(i,     custommult()));
  BOOST_CHECK_EQUAL(mpi_op_custommult_d,mimic_usage(d,     custommult()));
  BOOST_CHECK_EQUAL(mpi_op_custommult_o,mimic_usage(o,     custommult()));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( operations_registered_types_are_really_static )
{
  int i;
  double d;
  optest o;

  // check if re-registration does not alter the MPI_operations (avoid committing the same type over and over)
  BOOST_CHECK_EQUAL(mpi_op_customplus_i,mimic_usage(i,PE::customplus()));
  BOOST_CHECK_EQUAL(mpi_op_customplus_d,mimic_usage(d,PE::customplus()));
  BOOST_CHECK_EQUAL(mpi_op_customplus_o,mimic_usage(o,PE::customplus()));
  BOOST_CHECK_EQUAL(mpi_op_custommult_i,mimic_usage(i,     custommult()));
  BOOST_CHECK_EQUAL(mpi_op_custommult_d,mimic_usage(d,     custommult()));
  BOOST_CHECK_EQUAL(mpi_op_custommult_o,mimic_usage(o,     custommult()));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( built_in_operation_with_custom_datatype )
{
  optest o;
  // check if non built-in datatype does not fall back to built-in Operation
  BOOST_CHECK_NE( mimic_usage(o,PE::max()),         MPI_MAX);
  BOOST_CHECK_NE( mimic_usage(o,PE::min()),         MPI_MIN);
  BOOST_CHECK_NE( mimic_usage(o,PE::plus()),        MPI_SUM);
  BOOST_CHECK_NE( mimic_usage(o,PE::multiplies()),  MPI_PROD);
  BOOST_CHECK_NE( mimic_usage(o,PE::logical_and()), MPI_LAND);
  BOOST_CHECK_NE( mimic_usage(o,PE::logical_or()),  MPI_LOR);
  BOOST_CHECK_NE( mimic_usage(o,PE::logical_xor()), MPI_LXOR);
  BOOST_CHECK_NE( mimic_usage(o,PE::bitwise_and()), MPI_BAND);
  BOOST_CHECK_NE( mimic_usage(o,PE::bitwise_or()),  MPI_BOR);
  BOOST_CHECK_NE( mimic_usage(o,PE::bitwise_xor()), MPI_BXOR);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize )
{
  PEProcessSortedExecute(-1,CFinfo << "Proccess " << PE::Comm::instance().rank() << "/" << PE::Comm::instance().size() << " says good bye." << CFendl;);
  PE::Comm::instance().finalize();
  BOOST_CHECK_EQUAL( PE::Comm::instance().is_active() , false );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////



