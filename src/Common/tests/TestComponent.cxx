#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "Common/Log.hh"
#include "Common/Component.hh"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct Component_Fixture
{
  /// common setup for each test case
  Component_Fixture()
  {
    // int*    argc = &boost::unit_test::framework::master_test_suite().argc;
    // char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~Component_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Component_TestSuite, Component_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructors )
{
  // constructor with empty path
  Component dir1 ( "dir1" );
  BOOST_CHECK_EQUAL ( dir1.name() , "dir1" );
  BOOST_CHECK_EQUAL ( dir1.path().string() , "" );
  BOOST_CHECK_EQUAL ( dir1.full_path().string() , "dir1" );

//  CFinfo << dir1.name() << "\n" << CFendl;
//  CFinfo << dir1.path().string() << "\n" << CFendl;
//  CFinfo << dir1.full_path().string() << "\n" << CFendl;

  // constructor with passed path
  Component root ( "root", "/" );
  BOOST_CHECK_EQUAL ( root.name() , "root" );
  BOOST_CHECK_EQUAL ( root.path().string() , "/" );
  BOOST_CHECK_EQUAL ( root.full_path().string() , "//root" );

//  CFinfo << root.name() << "\n" << CFendl;
//  CFinfo << root.path().string() << "\n" << CFendl;
//  CFinfo << root.full_path().string() << "\n" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( add_component )
{
  Component* root = new Component ( "root", "/" );

  Component* dir1 =  new Component ( "dir1" );
  Component* dir2 =  new Component ( "dir2" );

  root->add_component( dir1 );
  dir1->add_component( dir2 );

  BOOST_CHECK_EQUAL ( root->full_path().string() , "//root" );
  BOOST_CHECK_EQUAL ( dir1->full_path().string() , "//root/dir1" );
  BOOST_CHECK_EQUAL ( dir2->full_path().string() , "//root/dir1/dir2" );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

