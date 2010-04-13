#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "Common/OSystem.hh"

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
  // BOOST_CHECK_NE( PE::interface().get_rank() , (Uint) 0 );
}


BOOST_AUTO_TEST_SUITE_END()
