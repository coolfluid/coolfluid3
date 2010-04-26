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
  Logger::getInstance().openFiles();
  
  // files are not created if PE is not initializaed
  if(PE::getInstance().is_init()) 
  {
    BOOST_CHECK(Logger::getInstance().getStream(Logger::INFO).isFileOpen());
    BOOST_CHECK(Logger::getInstance().getStream(Logger::ERROR).isFileOpen());
    BOOST_CHECK(Logger::getInstance().getStream(Logger::WARN).isFileOpen());
    BOOST_CHECK(Logger::getInstance().getStream(Logger::DEBUG).isFileOpen());
    BOOST_CHECK(Logger::getInstance().getStream(Logger::TRACE).isFileOpen());
  }
  else   
  {
    BOOST_CHECK(!Logger::getInstance().getStream(Logger::INFO).isFileOpen());
    BOOST_CHECK(!Logger::getInstance().getStream(Logger::ERROR).isFileOpen());
    BOOST_CHECK(!Logger::getInstance().getStream(Logger::WARN).isFileOpen());
    BOOST_CHECK(!Logger::getInstance().getStream(Logger::DEBUG).isFileOpen());
    BOOST_CHECK(!Logger::getInstance().getStream(Logger::TRACE).isFileOpen());
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_AUTO_TEST_SUITE_END()
