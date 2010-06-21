#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "Common/OSystem.hpp"

using namespace std;
using namespace CF;
using namespace CF::Common;

struct OSystem_Fixture
{
  /// common setup for each test case
  OSystem_Fixture()
  {
  }

  /// common tear-down for each test case
  ~OSystem_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here

};

BOOST_FIXTURE_TEST_SUITE( OSystem_TestSuite, OSystem_Fixture )

BOOST_AUTO_TEST_CASE( getProcessInfo )
{
  BOOST_CHECK( OSystem::instance().OSystemLayer().isNotNull() );
}

BOOST_AUTO_TEST_CASE( getSignalHandler )
{
  BOOST_CHECK( OSystem::instance().OSystemLayer().isNotNull() );
}

BOOST_AUTO_TEST_CASE( getLibLoader )
{
  BOOST_CHECK( OSystem::instance().LibLoader().isNotNull() );
}

BOOST_AUTO_TEST_CASE( executeCommand )
{
  /// @todo this test is not cross-platform
  // should exit normally
  BOOST_CHECK_NO_THROW( OSystem::instance().executeCommand("echo"));
  // the command does *normally* not exist, should throw an exception
  /// @todo find a command that throws an exception
  //BOOST_CHECK_THROW( OSystem::instance().executeCommand("cd /aDirThatDoesNotExist"), OSystemError);
}

BOOST_AUTO_TEST_SUITE_END()
