#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <iostream>

#include "Common/PropertyList.hpp"

using namespace std;
using namespace boost;

using namespace CF;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct Properties_Fixture
{
  /// common setup for each test case
  Properties_Fixture()
  {
    // int*    argc = &boost::unit_test::framework::master_test_suite().argc;
    // char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~Properties_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Properties_TestSuite, Properties_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( construct )
{
  PropertyList props;

  std::cout << "test construct" << std::endl;

  // test construction

  props.list["count"] = int ( 10 );
  props.list["name"]  = std::string( "lolo" );

  BOOST_CHECK_EQUAL ( props.check( "nono" ),  false );
  BOOST_CHECK_EQUAL ( props.check( "count" ), true );
  BOOST_CHECK_EQUAL ( props.check( "name" ),  true );

  // test no duplicates

  props.list["name"]  = std::string( "lolo" );

  BOOST_CHECK_EQUAL ( props.check( "name" ), 1 );

}

BOOST_AUTO_TEST_CASE( assign )
{
  PropertyList props;

  std::cout << "test assign" << std::endl;

  // test assign

  props.list["name"]  = std::string( "john" );

  BOOST_CHECK_EQUAL ( props.get<std::string>( "name" ), "john" );

  // test re-assign

  props.list["name"]  = std::string( "joanna" );

  BOOST_CHECK_EQUAL ( props.get<std::string>( "name" ), "joanna" );
}

BOOST_AUTO_TEST_CASE( list )
{
  PropertyList props;

  std::cout << "test list" << std::endl;

  // test construction

  props.list["count0"] = Uint ( 10 );
  props.list["count1"] = Uint ( 11 );
  props.list["count2"] = Uint ( 12 );
  props.list["count3"] = Uint ( 13 );

  Uint counter = 10;
  PropertyList::iterator itr = props.list.begin();
  for ( itr = props.list.begin(); itr != props.list.begin(); ++itr, ++counter )
  {
    const std::string& pname = itr->first;
    BOOST_CHECK_EQUAL ( props.get<Uint>( pname ), counter );
  }
}

BOOST_AUTO_TEST_CASE( remove )
{
  PropertyList props;

  std::cout << "test remove" << std::endl;

  props.list["count"]   = int ( 10 );
  props.list["name"]    = std::string( "john" );
  props.list["surname"] = std::string( "doe" );
  props.list["size"]    = int ( 99 );

  BOOST_CHECK_EQUAL ( props.check( "count" ),   true );
  BOOST_CHECK_EQUAL ( props.check( "name" ),    true );
  BOOST_CHECK_EQUAL ( props.check( "surname" ), true );
  BOOST_CHECK_EQUAL ( props.check( "size" ),    true );

  BOOST_CHECK_EQUAL ( props.check( "address" ), false );
  BOOST_CHECK_EQUAL ( props.check( "age" ),     false );

  // test removal of existing properties

  props.remove("count");
  props.remove("surname");

  BOOST_CHECK_EQUAL ( props.check( "count" ),   false );
  BOOST_CHECK_EQUAL ( props.check( "surname" ), false );

  // test removal of non-existing properties
  // should not throw or crash

  props.remove("address");
  BOOST_CHECK_EQUAL ( props.check( "address" ), false );

  props.remove("age");
  BOOST_CHECK_EQUAL ( props.check( "age" ),     false );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
