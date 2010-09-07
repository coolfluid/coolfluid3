#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "Common/CPath.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct CPath_Fixture
{
  /// common setup for each test case
  CPath_Fixture()
  {
    // int*    argc = &boost::unit_test::framework::master_test_suite().argc;
    // char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~CPath_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( CPath_TestSuite, CPath_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructors )
{
  // empty contructor
  CPath p0;
  BOOST_CHECK(p0.empty());
  BOOST_CHECK_EQUAL(p0.string().size(), (size_t)0 );

  // string constructor
  CPath p1 ( "lolo" );
  BOOST_CHECK(!p1.empty());
  BOOST_CHECK_EQUAL( std::strcmp( p1.string().c_str(), "lolo" ), 0 );

  // copy constructor
  CPath p2 ( "koko" );
  CPath p3 ( p2 );
  BOOST_CHECK(!p2.empty());
  BOOST_CHECK(!p3.empty());
  BOOST_CHECK_EQUAL( std::strcmp( p2.string().c_str(), p2.string().c_str() ), 0 );
  
  URI uri_absolute ( "cpath://hostname/root/component");
  URI uri_relative ( "../component");
  
  CPath p4(uri_absolute);
  BOOST_CHECK(!p4.empty());
  BOOST_CHECK_EQUAL( p4.string(), "//hostname/root/component");
  BOOST_CHECK(p4.is_absolute());
  
  CPath p5(uri_relative);
  BOOST_CHECK(!p5.empty());
  BOOST_CHECK_EQUAL( p5.string(), "../component");
  BOOST_CHECK(p5.is_relative());
  
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( concatenation )
{
  CPath p0 ( "/root/dir1" );
  CPath p1 ( "dir2/dir3" );

  CPath p2 = p0 / p1;
  BOOST_CHECK_EQUAL( std::strcmp( p2.string().c_str(), "/root/dir1/dir2/dir3" ), 0 );

  CPath p3;
  p3 /= p0;
  BOOST_CHECK_EQUAL( std::strcmp( p3.string().c_str(), "/root/dir1" ), 0 );

  CPath p5 = p0 / "dir5/dir55";
  BOOST_CHECK_EQUAL( std::strcmp( p5.string().c_str(), "/root/dir1/dir5/dir55" ), 0 );

  CPath p6 = "/root/dir6";
  BOOST_CHECK_EQUAL( std::strcmp( p6.string().c_str(), "/root/dir6" ), 0 );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

