// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

#include <iostream>

#include "Common/Log.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

struct LogFixture
{
  /// common setup for each test case
  LogFixture() { }
  
  /// common tear-down for each test case
  ~LogFixture() { }
  
  /// possibly common functions used on the tests below
  
  /// common values accessed by all tests goes here
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_FIXTURE_TEST_SUITE(LogTestSuite, LogFixture)

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_AUTO_TEST_CASE( openFiles )
{
  Logger::instance().openFiles();
  
  // files are not created if PE is not initializaed
  if(PEInterface::instance().is_init()) 
  {
    BOOST_CHECK(Logger::instance().getStream(Logger::INFO).isFileOpen());
    BOOST_CHECK(Logger::instance().getStream(Logger::ERROR).isFileOpen());
    BOOST_CHECK(Logger::instance().getStream(Logger::WARN).isFileOpen());
    BOOST_CHECK(Logger::instance().getStream(Logger::DEBUG).isFileOpen());
    BOOST_CHECK(Logger::instance().getStream(Logger::TRACE).isFileOpen());
  }
  else   
  {
    BOOST_CHECK(!Logger::instance().getStream(Logger::INFO).isFileOpen());
    BOOST_CHECK(!Logger::instance().getStream(Logger::ERROR).isFileOpen());
    BOOST_CHECK(!Logger::instance().getStream(Logger::WARN).isFileOpen());
    BOOST_CHECK(!Logger::instance().getStream(Logger::DEBUG).isFileOpen());
    BOOST_CHECK(!Logger::instance().getStream(Logger::TRACE).isFileOpen());
  }
}

/// Demonstrate the need to flush after writing an atomic log message
BOOST_AUTO_TEST_CASE( FlushBuffer )
{
  // Output order is not what we expect
  CFinfo << "1. this is unflused CFlog line 1" << "\n";
  std::cout << "2. This is flused std::cout" << std::endl;
  CFinfo << "3. this is flushed CFlog line 2" << CFendl;
  CFinfo << "---------------------------------" << CFendl;
  // Output order is correct
  CFinfo << "1. this is flused CFlog line 1" << CFendl;
  std::cout << "2. This is flused std::cout" << std::endl;
  CFinfo << "3. this is flushed CFlog line 2" << CFendl;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_AUTO_TEST_SUITE_END()
