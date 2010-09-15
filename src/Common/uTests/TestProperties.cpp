// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

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

  // test construction

  props["count"] = int ( 10 );
  props["name"]  = std::string( "lolo" );

  BOOST_CHECK_EQUAL ( props.check( "nono" ),  false );
  BOOST_CHECK_EQUAL ( props.check( "count" ), true );
  BOOST_CHECK_EQUAL ( props.check( "name" ),  true );

  // test no duplicates

  props["name"]  = std::string( "lolo" );

  BOOST_CHECK_EQUAL ( props.check( "name" ), 1 );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( assign )
{
  PropertyList props;

  // test assign

  props["name"]  = std::string( "john" );

  BOOST_CHECK_EQUAL ( props.value<std::string>( "name" ), "john" );

  // test re-assign

  props["name"]  = std::string( "joanna" );

  BOOST_CHECK_EQUAL ( props.value<std::string>( "name" ), "joanna" );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( list )
{
  PropertyList props;

  // test construction

  props["count0"] = Uint ( 10 );
  props["count1"] = Uint ( 11 );
  props["count2"] = Uint ( 12 );
  props["count3"] = Uint ( 13 );

  Uint counter = 10;
  PropertyList::iterator itr = props.begin();
  for ( itr = props.begin(); itr != props.begin(); ++itr, ++counter )
  {
    const std::string& pname = itr->first;
    BOOST_CHECK_EQUAL ( props.value<Uint>( pname ), counter );
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( remove )
{
  PropertyList props;

  props["count"]   = int ( 10 );
  props["name"]    = std::string( "john" );
  props["surname"] = std::string( "doe" );
  props["size"]    = int ( 99 );

  BOOST_CHECK_EQUAL ( props.size(), (Uint) 4 );

  BOOST_CHECK_EQUAL ( props.check( "count" ),   true );
  BOOST_CHECK_EQUAL ( props.check( "name" ),    true );
  BOOST_CHECK_EQUAL ( props.check( "surname" ), true );
  BOOST_CHECK_EQUAL ( props.check( "size" ),    true );

  BOOST_CHECK_EQUAL ( props.check( "address" ), false );
  BOOST_CHECK_EQUAL ( props.check( "age" ),     false );

  // test removal of existing properties

  props.erase("count");
  props.erase("surname");

  BOOST_CHECK_EQUAL ( props.size(), (Uint) 2 );

  BOOST_CHECK_EQUAL ( props.check( "count" ),   false );
  BOOST_CHECK_EQUAL ( props.check( "surname" ), false );

  // test removal of non-existing properties
  // should not throw or crash

  props.erase("address");
  BOOST_CHECK_EQUAL ( props.check( "address" ), false );

  props.erase("age");
  BOOST_CHECK_EQUAL ( props.check( "age" ),     false );

  BOOST_CHECK_EQUAL ( props.size(), (Uint) 2 );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
