// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for parallel fields"

#include <iomanip>
#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "common/Foreach.hpp"
#include "common/OSystem.hpp"
#include "common/OSystemLayer.hpp"

#include "common/PE/Comm.hpp"
#include "common/PE/Buffer.hpp"
#include "common/PE/debug.hpp"

using namespace boost;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::common::PE;

////////////////////////////////////////////////////////////////////////////////

struct MPIBufferTests_Fixture
{
  /// common setup for each test case
  MPIBufferTests_Fixture()
  {
    // uncomment if you want to use arguments to the test executable
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;

  }

  /// common tear-down for each test case
  ~MPIBufferTests_Fixture()
  {
  }

  /// possibly common functions used on the tests below
  int m_argc;
  char** m_argv;
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MPIBufferTests_TestSuite, MPIBufferTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
  Core::instance().initiate(m_argc,m_argv);
  PE::Comm::instance().init(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_pack_unpack )
{
  PE::Buffer buf;
  buf << 1u << 2u << 3.0 << 4 << true;
  BOOST_CHECK_EQUAL( buf.more_to_unpack(), true);

  Uint data_unsigned;
  Real data_real;
  int data_integer;
  std::vector<Real> data_array_real;
  std::string data_string;
  std::vector<std::string> data_array_string;
  bool data_bool;

  buf >> data_unsigned;  BOOST_CHECK_EQUAL(data_unsigned, 1u  );
  buf >> data_unsigned;  BOOST_CHECK_EQUAL(data_unsigned, 2u  );
  buf >> data_real;      BOOST_CHECK_EQUAL(data_real,     3.0 );
  buf >> data_integer;   BOOST_CHECK_EQUAL(data_integer,  4   );
  buf >> data_bool;      BOOST_CHECK_EQUAL(data_bool,     true);
  BOOST_CHECK_EQUAL( buf.more_to_unpack(), false);


  buf << 5;  buf >> data_integer; BOOST_CHECK_EQUAL (data_integer , 5);
  BOOST_CHECK_EQUAL( buf.more_to_unpack(), false);

  buf << std::vector<Real>(4,6.);
  buf >> data_array_real;

  BOOST_CHECK_EQUAL(data_array_real.size(), 4u);
  BOOST_CHECK_EQUAL(data_array_real[0], 6.);
  BOOST_CHECK_EQUAL(data_array_real[1], 6.);
  BOOST_CHECK_EQUAL(data_array_real[2], 6.);
  BOOST_CHECK_EQUAL(data_array_real[3], 6.);
  BOOST_CHECK_EQUAL( buf.more_to_unpack(), false);

  boost::multi_array<Real,2> table;
  table.resize(boost::extents[4][3]);
  table[2][0]=1.;
  table[2][1]=2.;
  table[2][2]=3.;

  buf << table[2];
  buf >> data_array_real;
  BOOST_CHECK_EQUAL(data_array_real[0], 1.);
  BOOST_CHECK_EQUAL(data_array_real[1], 2.);
  BOOST_CHECK_EQUAL(data_array_real[2], 3.);
  BOOST_CHECK_EQUAL( buf.more_to_unpack(), false);

  buf << std::string("a nice string");
  buf >> data_string;
  BOOST_CHECK_EQUAL(data_string, std::string("a nice string"));
  BOOST_CHECK_EQUAL(buf.more_to_unpack(), false);

  buf << std::vector<std::string>(3,"another nice string");
  buf >> data_array_string;
  BOOST_CHECK_EQUAL(data_array_string[0], std::string("another nice string"));
  BOOST_CHECK_EQUAL(data_array_string[1], std::string("another nice string"));
  BOOST_CHECK_EQUAL(data_array_string[2], std::string("another nice string"));
  BOOST_CHECK_EQUAL(buf.more_to_unpack(), false);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_broadcast )
{

  // Initialize some data, different for every processor

  int first = Comm::instance().rank();
  Real second = (1+static_cast<Real>(Comm::instance().rank()))*1e-6;
  bool third = (first == 0);
  std::string fourth = "from_proc_" + to_str(first);
  std::vector<int> fifth(3,first);


  Uint expected_size =
         sizeof(int) // first
      +  sizeof(Real) // second
      +  sizeof(Uint) // third
      +  sizeof(Uint)+sizeof(char)*fourth.size() // fourth
      +  sizeof(size_t)+sizeof(int)*fifth.size(); // fifth

  std::cout << "expected size is " << expected_size << std::endl;
//  Uint expected_size = 51;

  // ----------------------------------

  // Create a buffer
  common::PE::Buffer buffer;
  int root = 0;

  // pack the buffer on root processor
  if (Comm::instance().rank() == root)
    buffer << first << second << third << fourth << fifth;

  // broad cast the buffer from root processor
  buffer.broadcast(root);

  // unpack the buffer on other processors
  if (Comm::instance().rank() != root)
    buffer >> first >> second >> third >> fourth >> fifth;

  // ----------------------------------

  BOOST_CHECK_EQUAL(buffer.size(), expected_size);

  if (Comm::instance().rank() != root)
  {
    BOOST_CHECK_EQUAL(buffer.more_to_unpack(), false);
  }

  // The data on every processor should be from rank root
  BOOST_CHECK_EQUAL(first, 0);
  BOOST_CHECK_EQUAL(second, 1e-6);
  BOOST_CHECK_EQUAL(third, true);
  BOOST_CHECK_EQUAL(fourth, "from_proc_0");
  BOOST_CHECK_EQUAL(fifth[2], 0);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_all_gather )
{

  // Initialize some data, different for every processor

  std::string str = "from_proc_" + to_str(Comm::instance().rank());

  // ----------------------------------

  // Create a buffer
  common::PE::Buffer buffer;
  buffer << str;

  common::PE::Buffer out_buf;
  buffer.all_gather(out_buf);

  while (out_buf.more_to_unpack())
  {
    out_buf >> str;
    CFinfo << str << CFendl;
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  Comm::instance().finalize();
  Core::instance().terminate();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

