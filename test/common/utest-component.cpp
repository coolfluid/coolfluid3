// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::Component"

#include <boost/test/unit_test.hpp>

#include <boost/foreach.hpp>
#include <boost/iterator.hpp>

#include "common/Log.hpp"
#include "common/Component.hpp"
#include "common/FindComponents.hpp"
#include "common/Group.hpp"
#include "common/Link.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalFrame.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::common::XML;

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( Component_TestSuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructors )
{
  // constructor with passed path
  boost::shared_ptr<Component> root = boost::static_pointer_cast<Component>(allocate_component<Group>("root"));

  BOOST_CHECK_EQUAL ( root->name() , "root" );
  BOOST_CHECK_EQUAL ( root->uri().base_path().string() , "cpath:/" );
  BOOST_CHECK_EQUAL ( root->uri().string() , "cpath:/" );

  BOOST_CHECK_EQUAL ( root->properties().check("brief") , true );
  BOOST_CHECK_EQUAL ( root->properties().check("description") , true );

  // constructor with empty path
  boost::shared_ptr<Group> dir1 = allocate_component<Group>( "dir1" );

  BOOST_CHECK_EQUAL ( dir1->name() , "dir1" );
  BOOST_CHECK_EQUAL ( dir1->uri().base_path().string() , "cpath:/" );
  BOOST_CHECK_EQUAL ( dir1->uri().string() , "cpath:/" );

  // constructor with passed path
  boost::shared_ptr<Link> lnk = allocate_component<Link>( "lnk" );

  BOOST_CHECK_EQUAL ( lnk->name() , "lnk" );
  BOOST_CHECK_EQUAL ( lnk->uri().base_path().string() , "cpath:/" );
  BOOST_CHECK_EQUAL ( lnk->uri().string() , "cpath:/" );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( add_component )
{
  boost::shared_ptr<Component> root = boost::static_pointer_cast<Component>(allocate_component<Group>("root"));

  boost::shared_ptr<Component> dir1 = allocate_component<Group>( "dir1" );
  boost::shared_ptr<Component> dir2 = allocate_component<Group>( "dir2" );

  root->add_component( dir1 );
  dir1->add_component( dir2 );

  BOOST_CHECK_EQUAL ( root->uri().string() , "cpath:/" );
  BOOST_CHECK_EQUAL ( dir1->uri().string() , "cpath:/dir1" );
  BOOST_CHECK_EQUAL ( dir2->uri().string() , "cpath:/dir1/dir2" );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( get )
{
  boost::shared_ptr<Component> root = boost::static_pointer_cast<Component>(allocate_component<Group>("root"));

  boost::shared_ptr<Component> dir1 = allocate_component<Group>( "dir1" );
  boost::shared_ptr<Component> lnk1 = allocate_component<Link>( "lnk1" );

  // add child components to root
  root->add_component( dir1 );
  root->add_component( lnk1 );

  // point link to the dir1
  boost::shared_ptr<Link> p_lnk1 = boost::dynamic_pointer_cast<Link>(lnk1);
  p_lnk1->link_to(*dir1);

  // check that the root returns himself
  BOOST_CHECK_EQUAL ( follow_link(*root)->name(), "root" );
  BOOST_CHECK_EQUAL ( follow_link(*root)->uri().string(), "cpath:/" );

  // check that the link is sane
  BOOST_CHECK_EQUAL ( lnk1->name(), "lnk1" );
  BOOST_CHECK_EQUAL ( lnk1->uri().string(), "cpath:/lnk1" );

  // check that the link returns the dir1
  BOOST_CHECK_EQUAL ( follow_link(*lnk1)->name(), "dir1" );
  BOOST_CHECK_EQUAL ( follow_link(*lnk1)->uri().string(), "cpath:/dir1" );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( complete_path )
{
  boost::shared_ptr<Component> root = boost::static_pointer_cast<Component>(allocate_component<Group>("root"));

  boost::shared_ptr<Component> dir1 = allocate_component<Group>( "dir1" );
  boost::shared_ptr<Component> dir2 = allocate_component<Group>( "dir2" );
  boost::shared_ptr<Component> dir3 = allocate_component<Group>( "dir3" );

  // add child components to root
  root->add_component( dir1 );
  dir1->add_component( dir2 );
  dir2->add_component( dir3 );

  // test absolute & complete path
  URI p0 ( "cpath:/dir1" );
  dir2->complete_path( p0 );
  BOOST_CHECK_EQUAL ( p0.string(), "cpath:/dir1" );

  // test relative
  URI p10 ( "cpath:.." );
  dir2->complete_path( p10 );
  BOOST_CHECK_EQUAL ( p10.string(), "cpath:/dir1" );

  // test relative
  URI p11 ( "cpath:./" );
  dir2->complete_path( p11 );
  BOOST_CHECK_EQUAL ( p11.string(), "cpath:/dir1/dir2" );

  // test relative & complete path
  URI p12 ( "cpath:../../dir2" );
  dir3->complete_path( p12 );
  BOOST_CHECK_EQUAL ( p12.string(), "cpath:/dir1/dir2" );

  // test absolute & incomplete path
  URI p2 ( "cpath:/dir1/dir2/../dir2" );
  dir2->complete_path( p2 );
  BOOST_CHECK_EQUAL ( p2.string(), "cpath:/dir1/dir2" );

  // test absolute & multiple incomplete path
  URI p3 ( "cpath:/dir1/../dir1/../dir1/dir2/../../dir1/dir2" );
  dir2->complete_path( p3 );
  BOOST_CHECK_EQUAL ( p3.string(), "cpath:/dir1/dir2" );

  // test absolute & multiple incomplete path at end
  URI p4 ( "cpath:/dir1/dir2/dir3/../../" );
  dir2->complete_path( p4 );
  BOOST_CHECK_EQUAL ( p4.string(), "cpath:/dir1" );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( access_component )
{
  boost::shared_ptr<Component> root = boost::static_pointer_cast<Component>(allocate_component<Group>("root"));

  boost::shared_ptr<Component> dir1 = allocate_component<Group>( "dir1" );
  boost::shared_ptr<Component> dir2 = allocate_component<Group>( "dir2" );
  boost::shared_ptr<Component> dir21 = allocate_component<Group>( "dir21" );
  boost::shared_ptr<Component> dir22 = allocate_component<Group>( "dir22" );

  // add child components to root
  root->add_component( dir1 );
  dir1->add_component( dir2 );
  dir2->add_component( dir21 );
  dir2->add_component( dir22 );

  // test relative & complete path
  URI p0 ( "cpath:../dir21" );
  Handle<Component> cp0 = dir22->access_component( p0 );
  BOOST_CHECK_EQUAL ( cp0->uri().string(), "cpath:/dir1/dir2/dir21" );

  // test relative & complete path
  URI p1 ( "cpath:/dir1" );
  Handle<Component> cp1 = dir22->access_component( p1 );
  BOOST_CHECK_EQUAL ( cp1->uri().string(), "cpath:/dir1" );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( move_to )
{
  boost::shared_ptr<Component> root = boost::static_pointer_cast<Component>(allocate_component<Group>("root"));

  boost::shared_ptr<Component> dir1 = allocate_component<Group>( "dir1" );
  boost::shared_ptr<Component> dir2 = allocate_component<Group>( "dir2" );

  // add child components to root
  root->add_component( dir1 );
  dir1->add_component( dir2 );

  BOOST_CHECK_EQUAL ( dir2->uri().string(), "cpath:/dir1/dir2" );

  dir2->move_to( *root );

  BOOST_CHECK_EQUAL ( dir2->uri().string(), "cpath:/dir2" );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( problem )
{
  boost::shared_ptr<Component> root = allocate_component<Group> ( "Simulator" );

  Handle<Component> proot = root->access_component("cpath:/");

  BOOST_CHECK_EQUAL ( proot->uri().string(), "cpath:/" );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( create_subcomponents )
{
  boost::shared_ptr<Component> root = boost::static_pointer_cast<Component>(allocate_component<Group>("root"));
  Handle<Component> comp1 = root->create_component<Component>("comp1");
  comp1->create_component<Component>("comp1_1");
  comp1->create_component<Component>("comp1_2");

  BOOST_CHECK_EQUAL(find_component_with_name(*root, "comp1").name(),"comp1");
  BOOST_CHECK_EQUAL(find_component_recursively_with_name(*root, "comp1_1").name(),"comp1_1");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( create_component_signal )
{
  boost::shared_ptr<Component> root = allocate_component<Group> ( "croot" );

  SignalFrame sf("Signal", "/", "/");

  sf.set_option( "name", common::class_name<std::string>(), "MyMesh" );
  sf.set_option( "atype", common::class_name<std::string>(), "MeshReader" );
  sf.set_option( "ctype", common::class_name<std::string>(), "CGNS" );

//  XmlOps::print_xml_node( *doc.get() );
//  XmlOps::write_xml_node( *doc.get(),  "test.xml" );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( rename )
{
  boost::shared_ptr<Component> root = allocate_component<Group> ( "Simulator" );

  Handle<Component> c1 = root->create_component<Component>("c1");

  BOOST_CHECK_EQUAL ( c1->name(), "c1" );

  Handle<Component> c2 = root->create_component<Component>("c1");

  BOOST_CHECK_EQUAL ( c2->name(), "c1_1" );

  Handle<Component> c3 = root->create_component<Component>("c1");

  BOOST_CHECK_EQUAL ( c3->name(), "c1_2" );

  // turn off exception dumping
  ExceptionManager::instance().ExceptionDumps = false;

  BOOST_CHECK_NO_THROW (c2->rename("c1_3"));
  BOOST_CHECK(is_not_null(root->get_child("c1_3")));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( clear )
{
  boost::shared_ptr<Component> root = allocate_component<Group> ( "Simulator" );
  Handle<Component> c1 = root->create_component<Component>("c1");
  Handle<Component> c2 = root->create_component<Component>("c2");
  
  BOOST_CHECK(is_not_null(c1));
  BOOST_CHECK(is_not_null(c2));
  
  root->clear();

  BOOST_CHECK(is_null(c1));
  BOOST_CHECK(is_null(c2));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
