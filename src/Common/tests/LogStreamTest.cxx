#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "Common/Log.hh"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

struct LogStreamFixture
{
  /// common setup for each test case
  LogStreamFixture() {}

  /// common tear-down for each test case
  ~LogStreamFixture() {}

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here

};

BOOST_FIXTURE_TEST_SUITE(LogStreamTestSuite,LogStreamFixture)

BOOST_AUTO_TEST_CASE( setLogLevel )
{
  LogStream stream("TestStream");

  stream.setLogLevel(VERBOSE);
  stream.setLogLevel(LogStream::SCREEN, SILENT);

 // BOOST_CHECK_EQUAL( (int)stream.getLogLevel(LogStream::FILE), (int)VERBOSE);
  BOOST_CHECK_EQUAL( (int)stream.getLogLevel(LogStream::SCREEN), (int)SILENT);

}

BOOST_AUTO_TEST_SUITE_END()
