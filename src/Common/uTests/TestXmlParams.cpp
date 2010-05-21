#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/XmlHelpers.hpp"
#include "Common/BasicExceptions.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

/////////////////////////////////////////////////////////////////////////////////////

struct XmlParams_Fixture
{
  /// common setup for each test case
  XmlParams_Fixture()
  {
    // int*    argc = &boost::unit_test::framework::master_test_suite().argc;
    // char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~XmlParams_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here

};

/////////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( XmlParams_TestSuite, XmlParams_Fixture )

/////////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructors )
{
  std::string text = (
      "<MyC>"
      " <Params>"
      "  <OptBool>     1  </OptBool>"
      "  <OptInt>    134  </OptInt>"
      "  <OptStr>   lolo  </OptStr>"
      "  <Unused>   popo  </Unused>"
      "  <VecInt>  2 8 9  </VecInt>"
      "  <Comp>   CGroup  </Comp>"
      " </Params>"
      "</MyC>"
   );

  boost::shared_ptr<XmlDoc> doc = XmlOps::parse ( text );

  BOOST_CHECK_NO_THROW( XmlParams params ( *doc.get() ) );

}

/////////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructor_throws )
{
  std::string text = (
      "<MyC>"
      " <Lolo>"
      " </Lolo>"
      " <Polo>"
      " </Polo>"
      "</MyC>"
   );

  boost::shared_ptr<XmlDoc> doc = XmlOps::parse ( text );
  BOOST_CHECK_THROW( XmlParams params ( *doc.get() ) , Common::XmlError );
}

/////////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( get_value )
{
  std::string text = (
      "<Signal>"
      " <Params>"
      "  <OptBool>     1  </OptBool>"
      "  <OptInt>    134  </OptInt>"
      "  <OptStr>   lolo  </OptStr>"
      "  <Unused>   popo  </Unused>"
      "  <VecInt>  2 8 9  </VecInt>"
      "  <Comp>   CGroup  </Comp>"
      " </Params>"
      "</Signal>"
   );

  boost::shared_ptr<XmlDoc> doc = XmlOps::parse ( text );

  XmlParams params ( *doc.get() );

  BOOST_REQUIRE_EQUAL ( params.get_value<bool>("OptBool") , true );

  BOOST_REQUIRE_EQUAL ( params.get_value<Uint>("OptInt") , 134 );

  BOOST_REQUIRE_EQUAL ( params.get_value<std::string>("OptStr") , "lolo" );

//  std::vector<Uint> v;
//  v += 2,8,9;
//  std::vector<Uint> cv = params.get_value<Uint>("VecInt");
//  BOOST_CHECK_EQUAL_COLLECTIONS( v.begin(), v.end(), cv.begin(), cv.end() );

  BOOST_REQUIRE_EQUAL ( params.get_value<std::string>("Comp") , "CGroup" );

}
/////////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

/////////////////////////////////////////////////////////////////////////////////////
