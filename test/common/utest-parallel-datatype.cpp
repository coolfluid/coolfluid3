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
#define BOOST_TEST_MODULE "Test module for cf3::common 's parallel environment - part of checking datatype handling."

////////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/datatype.hpp"

#include "common/PE/debug.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct PEDatatypeFixture
{
  /// common setup for each test case
  PEDatatypeFixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// coemmon tear-down for each test case
  ~PEDatatypeFixture() { }

  /// common params
  int m_argc;
  char** m_argv;

  /// some complex data types
  struct user_struct_i
  {
    int i1,i2,i3,i4,i5;
  };
  struct user_struct_d
  {
    double d1,d2,d3,d4,d5,d6,d7,d8,d9;
  };
  struct user_struct_c
  {
    char c1,c2,c3,c4,c5,c6,c7;
  };
};

////////////////////////////////////////////////////////////////////////////////

/// data stays in scope for checking if registration is really static
PE::Datatype mpi_datatype_usi=nullptr;
PE::Datatype mpi_datatype_usd=nullptr;

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( PEDatatypeSuite, PEDatatypeFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  PE::Comm::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL( PE::Comm::instance().is_active() , true );
  PEProcessSortedExecute(-1,CFinfo << "Proccess " << PE::Comm::instance().rank() << "/" << PE::Comm::instance().size() << " reports in." << CFendl;);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( datatype_default_types )
{
  char test_char;
  BOOST_CHECK_EQUAL(PE::get_mpi_datatype(test_char),MPI_CHAR);
  unsigned char test_unsigned_char;
  BOOST_CHECK_EQUAL(PE::get_mpi_datatype(test_unsigned_char),MPI_UNSIGNED_CHAR);
  short test_short;
  BOOST_CHECK_EQUAL(PE::get_mpi_datatype(test_short),MPI_SHORT);
  unsigned short test_unsigned_short;
  BOOST_CHECK_EQUAL(PE::get_mpi_datatype(test_unsigned_short),MPI_UNSIGNED_SHORT);
  int test_int;
  BOOST_CHECK_EQUAL(PE::get_mpi_datatype(test_int),MPI_INT);
  unsigned int test_unsigned_int;
  BOOST_CHECK_EQUAL(PE::get_mpi_datatype(test_unsigned_int),MPI_UNSIGNED);
  long test_long;
  BOOST_CHECK_EQUAL(PE::get_mpi_datatype(test_long),MPI_LONG);
  unsigned long test_unsigned_long;
  BOOST_CHECK_EQUAL(PE::get_mpi_datatype(test_unsigned_long),MPI_UNSIGNED_LONG);
  long long test_long_long;
  BOOST_CHECK_EQUAL(PE::get_mpi_datatype(test_long_long),MPI_LONG_LONG);
  unsigned long long test_unsigned_long_long;
  BOOST_CHECK_EQUAL(PE::get_mpi_datatype(test_unsigned_long_long),MPI_UNSIGNED_LONG_LONG);
  float test_float;
  BOOST_CHECK_EQUAL(PE::get_mpi_datatype(test_float),MPI_FLOAT);
  double test_double;
  BOOST_CHECK_EQUAL(PE::get_mpi_datatype(test_double),MPI_DOUBLE);
  long double test_long_double;
  BOOST_CHECK_EQUAL(PE::get_mpi_datatype(test_long_double),MPI_LONG_DOUBLE);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( datatype_registered_types )
{
  // check if registering goes fine
  user_struct_i usi;
  mpi_datatype_usi=PE::get_mpi_datatype(usi);
  BOOST_CHECK_NE(mpi_datatype_usi,(PE::Datatype)nullptr);
  user_struct_d usd;
  mpi_datatype_usd=PE::get_mpi_datatype(usd);
  BOOST_CHECK_NE(mpi_datatype_usd,(PE::Datatype)nullptr);

  // check if no glitch and separate types go to separate static variables
  BOOST_CHECK_NE(mpi_datatype_usd,mpi_datatype_usi);

  // check if re-registration does not alter the Datatype (avoid committing the same type over and over)
  BOOST_CHECK_EQUAL(mpi_datatype_usi,PE::get_mpi_datatype(usi));
  BOOST_CHECK_EQUAL(mpi_datatype_usd,PE::get_mpi_datatype(usd));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( datatype_registered_types_are_really_static )
{
  // check if re-registration does not alter the Datatype (avoid committing the same type over and over)
  user_struct_i usi;
  BOOST_CHECK_EQUAL(mpi_datatype_usi,PE::get_mpi_datatype(usi));
  user_struct_d usd;
  BOOST_CHECK_EQUAL(mpi_datatype_usd,PE::get_mpi_datatype(usd));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( internal_mechanism_returns_nullptr_for_non_registered_types )
{
  // detail::get_mpi
  // this is just to be sure that nothing fishy happens if data type is unknown,
  // normally you shouldn't call ::detail functions
  user_struct_c usc;
  BOOST_CHECK_EQUAL(PE::detail::get_mpi_datatype_impl(usc),(PE::Datatype)nullptr);
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


