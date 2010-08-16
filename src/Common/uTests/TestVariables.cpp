#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

// #include "Common/Variables.hpp"

using namespace std;
using namespace boost;

// using namespace CF;
// using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct Variables_Fixture
{
  /// common setup for each test case
  Variables_Fixture()
  {
    // int*    argc = &boost::unit_test::framework::master_test_suite().argc;
    // char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~Variables_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Variables_TestSuite, Variables_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( construct )
{
}

BOOST_AUTO_TEST_CASE( assign )
{
}

BOOST_AUTO_TEST_CASE( list )
{
}

BOOST_AUTO_TEST_CASE( remove )
{
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
