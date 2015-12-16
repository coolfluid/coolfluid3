// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF log level filter"

#include <boost/test/unit_test.hpp>

#include <boost/iostreams/device/back_inserter.hpp>

#include <iostream>

#include "common/LogLevelFilter.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::common;

struct LogLevelFilterFixture
{
  /// common setup for each test case
  LogLevelFilterFixture() :
  m_buffer(),
  m_sink(iostreams::back_inserter(m_buffer))
  {
    m_filter = new LogLevelFilter(INFO);
  }

  /// common tear-down for each test case
  ~LogLevelFilterFixture() { delete m_filter; }

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here
  LogLevelFilter * m_filter;

  string m_buffer;

  iostreams::back_insert_device<string> m_sink;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_FIXTURE_TEST_SUITE(LogLevelFilterTestSuite,LogLevelFilterFixture)

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_AUTO_TEST_CASE( setLogLevel )
{
  LogLevelFilterFixture f;

  f.m_filter->set_filter(SILENT);

  BOOST_CHECK_EQUAL( (int)f.m_filter->get_filter(), (int)SILENT);

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_AUTO_TEST_CASE( write )
{
  /// test whether the string is forwarded
  LogLevelFilterFixture f;
  string str = "Hello world!";

  f.m_filter->set_filter(INFO);

  f.m_filter->set_log_level(SILENT);
  f.m_filter->write(f.m_sink, str.c_str(), str.length());
  BOOST_CHECK_EQUAL(f.m_buffer, std::string(""));

  f.m_filter->set_log_level(ERROR);
  f.m_filter->write(f.m_sink, str.c_str(), str.length());
  BOOST_CHECK_EQUAL(f.m_buffer, std::string(""));

  f.m_filter->set_log_level(WARNING);
  f.m_filter->write(f.m_sink, str.c_str(), str.length());
  BOOST_CHECK_EQUAL(f.m_buffer, std::string(""));

  f.m_filter->set_log_level(INFO);
  f.m_buffer.clear();
  f.m_filter->write(f.m_sink, str.c_str(), str.length());
  BOOST_CHECK_EQUAL(str, f.m_buffer);

  f.m_filter->set_log_level(DEBUG);
  f.m_buffer.clear();
  f.m_filter->write(f.m_sink, str.c_str(), str.length());
  BOOST_CHECK_EQUAL(str, f.m_buffer);

  f.m_buffer.clear();
  f.m_filter->write(f.m_sink, str.c_str(), str.length());
  BOOST_CHECK_EQUAL(str, f.m_buffer);

  f.m_filter->set_log_level(VERBOSE);
  f.m_buffer.clear();
  f.m_filter->write(f.m_sink, str.c_str(), str.length());
  BOOST_CHECK_EQUAL(str, f.m_buffer);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_AUTO_TEST_SUITE_END()
