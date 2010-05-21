#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/XML.hpp"
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

  XmlParser parser ( text );

  boost::shared_ptr<XmlDoc> doc = parser.xmldoc;

  XmlParams params ( parser.getXml() );

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

  XmlParser parser ( text );
  BOOST_CHECK_THROW( XmlParams params ( parser.getXml() ) , Common::XmlError );
}

/////////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

/////////////////////////////////////////////////////////////////////////////////////
