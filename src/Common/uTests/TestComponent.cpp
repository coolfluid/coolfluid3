#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/Component.hpp"
#include "Common/CRoot.hpp"
#include "Common/CGroup.hpp"
#include "Common/CLink.hpp"

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
  // constructor with passed path
  boost::shared_ptr<CRoot> root = CRoot::create ( "root" );
  BOOST_CHECK_EQUAL ( root->name() , "root" );
  BOOST_CHECK_EQUAL ( root->path().string() , "/" );
  BOOST_CHECK_EQUAL ( root->full_path().string() , "//root" );

  // constructor with empty path
  CGroup dir1 ( "dir1" );
  BOOST_CHECK_EQUAL ( dir1.name() , "dir1" );
  BOOST_CHECK_EQUAL ( dir1.path().string() , "" );
  BOOST_CHECK_EQUAL ( dir1.full_path().string() , "dir1" );

  // constructor with passed path
  CLink lnk ( "lnk" );
  BOOST_CHECK_EQUAL ( lnk.name() , "lnk" );
  BOOST_CHECK_EQUAL ( lnk.path().string() , "" );
  BOOST_CHECK_EQUAL ( lnk.full_path().string() , "lnk" );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( add_component )
{
  boost::shared_ptr<CRoot> root = CRoot::create ( "root" );

  boost::shared_ptr<Component> dir1 ( new CGroup ( "dir1" ) );
  boost::shared_ptr<Component> dir2 ( new CGroup ( "dir2" ) );

  root->add_component( dir1 );
  dir1->add_component( dir2 );

  BOOST_CHECK_EQUAL ( root->full_path().string() , "//root" );
  BOOST_CHECK_EQUAL ( dir1->full_path().string() , "//root/dir1" );
  BOOST_CHECK_EQUAL ( dir2->full_path().string() , "//root/dir1/dir2" );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( xml_tree )
{
//  boost::shared_ptr<CRoot> root = CRoot::create ( "root" );
//
//  boost::shared_ptr<Component> dir1 ( new CGroup ( "dir1" ) );
//  boost::shared_ptr<Component> dir2 ( new CGroup ( "dir2" ) );
//
//  root->add_component( dir1 );
//  dir1->add_component( dir2 );
//
//  XMLNode root_node = XMLNode::createXMLTopNode("xml", TRUE);
//
//  root_node.addAttribute("version","1.0");
//  root_node.addAttribute("encoding","UTF-8");
//  root_node.addAttribute("standalone","yes");
//
//  root->xml_tree( root_node );
//
//  XMLSTR xml_str = root_node.createXMLString();
//
////  CFinfo << "xml_str\n" << xml_str << CFendl;
//
//  freeXMLString(xml_str);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( is_link )
{
  boost::shared_ptr<CRoot> root = CRoot::create ( "root" );

  boost::shared_ptr<Component> dir1 ( new CGroup ( "dir1" ) );

  root->add_component( dir1 );

  BOOST_CHECK ( ! root->is_link() );
  BOOST_CHECK ( ! dir1->is_link() );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( get )
{
  boost::shared_ptr<CRoot> root = CRoot::create ( "root" );

  boost::shared_ptr<Component> dir1 ( new CGroup ( "dir1" ) );
  boost::shared_ptr<Component> lnk1 ( new CLink  ( "lnk1" ) );

  // add child components to root
  root->add_component( dir1 );
  root->add_component( lnk1 );

  // point link to the dir1
  boost::shared_ptr<CLink> p_lnk1 = boost::dynamic_pointer_cast<CLink>(lnk1);
  p_lnk1->link_to(dir1);

  // check that the root returns himself
  BOOST_CHECK_EQUAL ( root->get()->name(), "root" );
  BOOST_CHECK_EQUAL ( root->get()->full_path().string(), "//root" );

  // check that the link is sane
  BOOST_CHECK_EQUAL ( lnk1->name(), "lnk1" );
  BOOST_CHECK_EQUAL ( lnk1->full_path().string(), "//root/lnk1" );

  // check that the link returns the dir1
  BOOST_CHECK_EQUAL ( lnk1->get()->name(), "dir1" );
  BOOST_CHECK_EQUAL ( lnk1->get()->full_path().string(), "//root/dir1" );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( complete_path )
{
  boost::shared_ptr<CRoot> root = CRoot::create ( "root" );

  boost::shared_ptr<Component> dir1 ( new CGroup ( "dir1" ) );
  boost::shared_ptr<Component> dir2 ( new CGroup ( "dir2" ) );
  boost::shared_ptr<Component> dir3 ( new CGroup ( "dir3" ) );

  // add child components to root
  root->add_component( dir1 );
  dir1->add_component( dir2 );
  dir2->add_component( dir3 );

  // test absolute & complete path
  CPath p0 ( "//root/dir1" );
  dir2->complete_path( p0 );
  BOOST_CHECK_EQUAL ( p0.string(), "//root/dir1" );

  // test relative
  CPath p10 ( ".." );
  dir2->complete_path( p10 );
  BOOST_CHECK_EQUAL ( p10.string(), "//root/dir1" );

  // test relative
  CPath p11 ( "./" );
  dir2->complete_path( p11 );
  BOOST_CHECK_EQUAL ( p11.string(), "//root/dir1/dir2" );

  // test relative & complete path
  CPath p12 ( "../../dir2" );
  dir3->complete_path( p12 );
  BOOST_CHECK_EQUAL ( p12.string(), "//root/dir1/dir2" );

  // test absolute & incomplete path
  CPath p2 ( "//root/dir1/dir2/../dir2" );
  dir2->complete_path( p2 );
  BOOST_CHECK_EQUAL ( p2.string(), "//root/dir1/dir2" );

  // test absolute & multiple incomplete path
  CPath p3 ( "//root/dir1/../dir2/../dir1/../dir2/dir3" );
  dir2->complete_path( p3 );
  BOOST_CHECK_EQUAL ( p3.string(), "//root/dir2/dir3" );

  // test absolute & multiple incomplete path at end
  CPath p4 ( "//root/dir1/dir2/dir3/../../" );
  dir2->complete_path( p4 );
  BOOST_CHECK_EQUAL ( p4.string(), "//root/dir1" );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( look_component )
{
  boost::shared_ptr<CRoot> root = CRoot::create ( "root" );

  boost::shared_ptr<Component> dir1  ( new CGroup ( "dir1" ) );
  boost::shared_ptr<Component> dir2  ( new CGroup ( "dir2" ) );
  boost::shared_ptr<Component> dir21 ( new CGroup ( "dir21" ) );
  boost::shared_ptr<Component> dir22 ( new CGroup ( "dir22" ) );

  // add child components to root
  root->add_component( dir1 );
  dir1->add_component( dir2 );
  dir2->add_component( dir21 );
  dir2->add_component( dir22 );

  // test relative & complete path
  CPath p0 ( "../dir21" );
  boost::shared_ptr<Component> cp0 = dir22->look_component( p0 );
  BOOST_CHECK_EQUAL ( cp0->full_path().string(), "//root/dir1/dir2/dir21" );

  // test relative & complete path
  CPath p1 ( "//root/dir1" );
  boost::shared_ptr<Component> cp1 = dir22->look_component( p1 );
  BOOST_CHECK_EQUAL ( cp1->full_path().string(), "//root/dir1" );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( change_parent )
{
  boost::shared_ptr<CRoot> root = CRoot::create ( "root" );

  boost::shared_ptr<Component> dir1  ( new CGroup ( "dir1" ) );
  boost::shared_ptr<Component> dir2  ( new CGroup ( "dir2" ) );

  // add child components to root
  root->add_component( dir1 );
  dir1->add_component( dir2 );

  BOOST_CHECK_EQUAL ( dir2->full_path().string(), "//root/dir1/dir2" );

  dir2->change_parent( root );

  BOOST_CHECK_EQUAL ( dir2->full_path().string(), "//root/dir2" );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( problem )
{
  boost::shared_ptr<CRoot> root = CRoot::create ( "Simulator" );

  boost::shared_ptr<Component> proot = root->look_component("//Simulator");

  BOOST_CHECK_EQUAL ( proot->full_path().string(), "//Simulator" );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

