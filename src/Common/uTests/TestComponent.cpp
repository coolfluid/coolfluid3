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
  boost::shared_ptr<Component> root ( new Component ( "root", "/" ) );
  boost::shared_ptr<Component> dir1 ( new Component ( "dir1" ) );
  boost::shared_ptr<Component> dir2 ( new Component ( "dir2" ) );

  root->add_component( dir1 );
  dir1->add_component( dir2 );

  BOOST_CHECK_EQUAL ( root->full_path().string() , "//root" );
  BOOST_CHECK_EQUAL ( dir1->full_path().string() , "//root/dir1" );
  BOOST_CHECK_EQUAL ( dir2->full_path().string() , "//root/dir1/dir2" );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( xml_tree )
{
  boost::shared_ptr<Component> root ( new Component ( "root", "/" ) );
  boost::shared_ptr<Component> dir1 ( new Component ( "dir1" ) );
  boost::shared_ptr<Component> dir2 ( new Component ( "dir2" ) );

  root->add_component( dir1 );
  dir1->add_component( dir2 );

  XMLNode root_node = XMLNode::createXMLTopNode("xml", TRUE);

  root_node.addAttribute("version","1.0");
  root_node.addAttribute("encoding","UTF-8");
  root_node.addAttribute("standalone","yes");

  root->xml_tree( root_node );

  XMLSTR xml_str = root_node.createXMLString();

//  CFinfo << "xml_str\n" << xml_str << CFendl;

  freeXMLString(xml_str);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( is_link )
{
  boost::shared_ptr<Component> root ( new Component ( "root", "/" ) );
  boost::shared_ptr<Component> dir1 ( new Component ( "dir1" ) );

  root->add_component( dir1 );

  BOOST_CHECK ( ! root->is_link() );
  BOOST_CHECK ( ! dir1->is_link() );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( get )
{
  boost::shared_ptr<Component> root ( new CRoot  ( "root" ) );
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

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

