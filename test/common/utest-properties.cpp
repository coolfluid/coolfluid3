// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for properties facility"

#include <boost/test/unit_test.hpp>

#include "common/Assertions.hpp"
#include "common/BasicExceptions.hpp"
#include "common/PropertyList.hpp"

using namespace std;
using namespace boost;

using namespace cf3;
using namespace cf3::common;

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

  props.add("count", int(10));
  props.add("name", std::string("lolo"));

  BOOST_CHECK_EQUAL ( props.check( "nono" ),  false );
  BOOST_CHECK_EQUAL ( props.check( "count" ), true );
  BOOST_CHECK_EQUAL ( props.check( "name" ),  true );

  // test no duplicates

//  props["name"]  = std::string( "lolo" );

//  BOOST_CHECK_EQUAL ( props.check( "name" ), 1 );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( assign )
{
  PropertyList props;

  props.add("name", std::string("(empty)"));

  // test assign

  props.set("name", std::string( "john" ));

  BOOST_CHECK_EQUAL ( props.value<std::string>("name"), "john" );

  // test re-assign

  props.set("name", std::string( "joanna" ));

  BOOST_CHECK_EQUAL ( props.value<std::string>("name"), "joanna" );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( list )
{
  PropertyList props;

  // test construction

  props.add("count0", Uint(10));
  props.add("count1", Uint(11));
  props.add("count2", Uint(12));
  props.add("count3", Uint(13));

  Uint counter = 10;
  PropertyList::PropertyStorage_t::iterator itr = props.store.begin();
  for ( ; itr != props.store.end(); ++itr, ++counter )
  {
    const std::string& pname = itr->first;
    BOOST_CHECK_EQUAL ( props.value<Uint>(pname), counter );
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( remove )
{
  AssertionManager::instance().AssertionThrows = true;
  PropertyList props;

  props.add("count", int(10));
  props.add("name", std::string("john"));
  props.add("surname", std::string("doe"));
  props.add("size", int(99));

  BOOST_CHECK_EQUAL ( props.store.size(), (Uint) 4 );

  BOOST_CHECK_EQUAL ( props.check( "count" ),   true );
  BOOST_CHECK_EQUAL ( props.check( "name" ),    true );
  BOOST_CHECK_EQUAL ( props.check( "surname" ), true );
  BOOST_CHECK_EQUAL ( props.check( "size" ),    true );

  BOOST_CHECK_EQUAL ( props.check( "address" ), false );
  BOOST_CHECK_EQUAL ( props.check( "age" ),     false );

  // test removal of existing properties

  props.erase("count");
  props.erase("surname");

  BOOST_CHECK_EQUAL ( props.store.size(), (Uint) 2 );

  BOOST_CHECK_EQUAL ( props.check( "count" ),   false );
  BOOST_CHECK_EQUAL ( props.check( "surname" ), false );

  // test removal of non-existing properties
  // should not throw or crash

  BOOST_CHECK_THROW ( props.erase("address"), ValueNotFound);
  BOOST_CHECK_EQUAL ( props.check( "address" ), false );

  BOOST_CHECK_THROW(props.erase("age"), ValueNotFound);
  BOOST_CHECK_EQUAL ( props.check( "age" ),     false );

  BOOST_CHECK_EQUAL ( props.store.size(), (Uint) 2 );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
