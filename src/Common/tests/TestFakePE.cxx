#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "Common/FakePE.hh"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

struct FakePE_Fixture
{
  /// common setup for each test case
  FakePE_Fixture() {}

  /// common tear-down for each test case
  ~FakePE_Fixture() {}

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here

};

BOOST_FIXTURE_TEST_SUITE( FakePE_TestSuite, FakePE_Fixture )

BOOST_AUTO_TEST_CASE( get_rank )
{
  BOOST_CHECK_EQUAL( FakePE::get_instance().get_rank() , 0 );
}

BOOST_AUTO_TEST_SUITE_END()
