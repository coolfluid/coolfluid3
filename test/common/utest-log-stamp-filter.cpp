// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF log stamp filter"

#include <boost/test/unit_test.hpp>

#include <boost/iostreams/device/back_inserter.hpp>

#include "common/LogStampFilter.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct LogStampFilter_Fixture
{
  /// common setup for each test case
  LogStampFilter_Fixture() :
  m_buffer(),
  m_sink(iostreams::back_inserter(m_buffer))
  {
    m_filter = new LogStampFilter("TestStream");
  }

  /// common tear-down for each test case
  ~LogStampFilter_Fixture() { delete m_filter; }

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here
  LogStampFilter * m_filter;

  string m_buffer;

  iostreams::back_insert_device<string> m_sink;
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( LogStampFilter_TestSuite, LogStampFilter_Fixture )

////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE( write )
{
  /// test whether the string is forwarded
  LogStampFilter_Fixture f;
  string str = "Hello world!";
  Uint nbbytes_wrtitten;

  nbbytes_wrtitten = f.m_filter->write(f.m_sink, str.c_str(), str.length());
  f.m_filter->endMessage();
  BOOST_CHECK_EQUAL(str, f.m_buffer);
  BOOST_CHECK_EQUAL(nbbytes_wrtitten, f.m_buffer.length());

  f.m_buffer.clear();

  // test the stamps ("TestStream" is the name of the filter created by
  // LogStampFilter_Fixture class)
  f.m_filter->setStamp("<%type%> ");
  nbbytes_wrtitten = f.m_filter->write(f.m_sink, str.c_str(), str.length());
  f.m_filter->endMessage();
  BOOST_CHECK_EQUAL(string("<TestStream> ") + str, f.m_buffer);
  // below : +13 because of the stamp size
  BOOST_CHECK_EQUAL(nbbytes_wrtitten, str.length() + 13);

  f.m_buffer.clear();

  // test the stamps ("TestStream" is the name of the filter created by
  // LogStampFilter_Fixture class)
  f.m_filter->setStamp("<%tpye%> ");
  nbbytes_wrtitten = f.m_filter->write(f.m_sink, str.c_str(), str.length());
  f.m_filter->endMessage();
  BOOST_CHECK_EQUAL(string("<%tpye%> ") + str, f.m_buffer);
  // below : +9 because of the stamp size
  BOOST_CHECK_EQUAL(nbbytes_wrtitten, str.length() + 9);

}

////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE( setPlace )
{
  LogStampFilter_Fixture f;
  CodeLocation cl = FromHere();
  string str = "Hello world!";
  unsigned int nbbytes_wrtitten;

  f.m_filter->setStamp("--%place%-- ");
  f.m_filter->setPlace(cl);
  nbbytes_wrtitten = f.m_filter->write(f.m_sink, str.c_str(), str.length());
  f.m_filter->endMessage();
  BOOST_CHECK_EQUAL(string("--") + cl.short_str() + string("-- ") + str, f.m_buffer);
  BOOST_CHECK_EQUAL(nbbytes_wrtitten, str.length() + cl.short_str().length() + 5);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
