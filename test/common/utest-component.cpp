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
#include "common/CRoot.hpp"
#include "common/Group.hpp"
#include "common/CLink.hpp"

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
  CRoot::Ptr root = CRoot::create ( "root" );

  BOOST_CHECK_EQUAL ( root->name() , "root" );
  BOOST_CHECK_EQUAL ( root->uri().base_path().string() , "cpath:/" );
  BOOST_CHECK_EQUAL ( root->uri().string() , "cpath://root" );

  BOOST_CHECK_EQUAL ( root->properties().check("brief") , true );
  BOOST_CHECK_EQUAL ( root->properties().check("description") , true );

  // constructor with empty path
  Group::Ptr dir1 = allocate_component<Group>( "dir1" );

  BOOST_CHECK_EQUAL ( dir1->name() , "dir1" );
  BOOST_CHECK_EQUAL ( dir1->uri().base_path().string() , "cpath:./" );
  BOOST_CHECK_EQUAL ( dir1->uri().string() , "cpath:dir1" );

  // constructor with passed path
  CLink::Ptr lnk = allocate_component<CLink>( "lnk" );

  BOOST_CHECK_EQUAL ( lnk->name() , "lnk" );
  BOOST_CHECK_EQUAL ( lnk->uri().base_path().string() , "cpath:./" );
  BOOST_CHECK_EQUAL ( lnk->uri().string() , "cpath:lnk" );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( add_component )
{
  CRoot::Ptr root = CRoot::create ( "root" );

  Component::Ptr dir1 = allocate_component<Group>( "dir1" );
  Component::Ptr dir2 = allocate_component<Group>( "dir2" );

  root->add_component( dir1 );
  dir1->add_component( dir2 );

  BOOST_CHECK_EQUAL ( root->uri().string() , "cpath://root" );
  BOOST_CHECK_EQUAL ( dir1->uri().string() , "cpath://root/dir1" );
  BOOST_CHECK_EQUAL ( dir2->uri().string() , "cpath://root/dir1/dir2" );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( is_link )
{
  CRoot::Ptr root = CRoot::create ( "root" );

  Component::Ptr dir1 = allocate_component<Group>( "dir1" );

  root->add_component( dir1 );

  BOOST_CHECK ( ! root->is_link() );
  BOOST_CHECK ( ! dir1->is_link() );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( get )
{
  CRoot::Ptr root = CRoot::create ( "root" );

  Component::Ptr dir1 = allocate_component<Group>( "dir1" );
  Component::Ptr lnk1 = allocate_component<CLink>( "lnk1" );

  // add child components to root
  root->add_component( dir1 );
  root->add_component( lnk1 );

  // point link to the dir1
  boost::shared_ptr<CLink> p_lnk1 = boost::dynamic_pointer_cast<CLink>(lnk1);
  p_lnk1->link_to(dir1);

  // check that the root returns himself
  BOOST_CHECK_EQUAL ( root->follow()->name(), "root" );
  BOOST_CHECK_EQUAL ( root->follow()->uri().string(), "cpath://root" );

  // check that the link is sane
  BOOST_CHECK_EQUAL ( lnk1->name(), "lnk1" );
  BOOST_CHECK_EQUAL ( lnk1->uri().string(), "cpath://root/lnk1" );

  // check that the link returns the dir1
  BOOST_CHECK_EQUAL ( lnk1->follow()->name(), "dir1" );
  BOOST_CHECK_EQUAL ( lnk1->follow()->uri().string(), "cpath://root/dir1" );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( as_const )
{
  CRoot::Ptr root = CRoot::create ( "root" );
  CRoot::ConstPtr const_root = root->as_const()->as_ptr<CRoot>();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( complete_path )
{
  CRoot::Ptr root = CRoot::create ( "root" );

  Component::Ptr dir1 = allocate_component<Group>( "dir1" );
  Component::Ptr dir2 = allocate_component<Group>( "dir2" );
  Component::Ptr dir3 = allocate_component<Group>( "dir3" );

  // add child components to root
  root->add_component( dir1 );
  dir1->add_component( dir2 );
  dir2->add_component( dir3 );

  // test absolute & complete path
  URI p0 ( "cpath://root/dir1" );
  dir2->complete_path( p0 );
  BOOST_CHECK_EQUAL ( p0.string(), "cpath://root/dir1" );

  // test relative
  URI p10 ( "cpath:.." );
  dir2->complete_path( p10 );
  BOOST_CHECK_EQUAL ( p10.string(), "cpath://root/dir1" );

  // test relative
  URI p11 ( "cpath:./" );
  dir2->complete_path( p11 );
  BOOST_CHECK_EQUAL ( p11.string(), "cpath://root/dir1/dir2" );

  // test relative & complete path
  URI p12 ( "cpath:../../dir2" );
  dir3->complete_path( p12 );
  BOOST_CHECK_EQUAL ( p12.string(), "cpath://root/dir1/dir2" );

  // test absolute & incomplete path
  URI p2 ( "cpath://root/dir1/dir2/../dir2" );
  dir2->complete_path( p2 );
  BOOST_CHECK_EQUAL ( p2.string(), "cpath://root/dir1/dir2" );

  // test absolute & multiple incomplete path
  URI p3 ( "cpath://root/dir1/../dir2/../dir1/../dir2/dir3" );
  dir2->complete_path( p3 );
  BOOST_CHECK_EQUAL ( p3.string(), "cpath://root/dir2/dir3" );

  // test absolute & multiple incomplete path at end
  URI p4 ( "cpath://root/dir1/dir2/dir3/../../" );
  dir2->complete_path( p4 );
  BOOST_CHECK_EQUAL ( p4.string(), "cpath://root/dir1" );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( access_component_ptr )
{
  CRoot::Ptr root = CRoot::create ( "root" );

  Component::Ptr dir1 = allocate_component<Group>( "dir1" );
  Component::Ptr dir2 = allocate_component<Group>( "dir2" );
  Component::Ptr dir21 = allocate_component<Group>( "dir21" );
  Component::Ptr dir22 = allocate_component<Group>( "dir22" );

  // add child components to root
  root->add_component( dir1 );
  dir1->add_component( dir2 );
  dir2->add_component( dir21 );
  dir2->add_component( dir22 );

  // test relative & complete path
  URI p0 ( "cpath:../dir21" );
  Component::Ptr cp0 = dir22->access_component_ptr( p0 );
  BOOST_CHECK_EQUAL ( cp0->uri().string(), "cpath://root/dir1/dir2/dir21" );

  // test relative & complete path
  URI p1 ( "cpath://root/dir1" );
  Component::Ptr cp1 = dir22->access_component_ptr( p1 );
  BOOST_CHECK_EQUAL ( cp1->uri().string(), "cpath://root/dir1" );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( move_to )
{
  CRoot::Ptr root = CRoot::create ( "root" );

  Component::Ptr dir1 = allocate_component<Group>( "dir1" );
  Component::Ptr dir2 = allocate_component<Group>( "dir2" );

  // add child components to root
  root->add_component( dir1 );
  dir1->add_component( dir2 );

  BOOST_CHECK_EQUAL ( dir2->uri().string(), "cpath://root/dir1/dir2" );

  dir2->move_to( *root );

  BOOST_CHECK_EQUAL ( dir2->uri().string(), "cpath://root/dir2" );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( problem )
{
  CRoot::Ptr root = CRoot::create ( "Simulator" );

  Component::Ptr proot = root->access_component_ptr("cpath://Simulator");

  BOOST_CHECK_EQUAL ( proot->uri().string(), "cpath://Simulator" );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( create_subcomponents )
{
  CRoot::Ptr root = CRoot::create ( "root" );
  Component::Ptr comp1 = root->create_component_ptr<Component>("comp1");
  comp1->create_component_ptr<Component>("comp1_1");
  comp1->create_component_ptr<Component>("comp1_2");

  BOOST_CHECK_EQUAL(find_component_with_name(*root, "comp1").name(),"comp1");
  BOOST_CHECK_EQUAL(find_component_recursively_with_name(*root, "comp1_1").name(),"comp1_1");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( create_component_signal )
{
  CRoot::Ptr root = CRoot::create ( "croot" );

  SignalFrame sf("Signal", "//Root", "//Root");

  sf.set_option<std::string>( "name",  "MyMesh" );
  sf.set_option<std::string>( "atype", "CMeshReader" );
  sf.set_option<std::string>( "ctype", "CGNS" );

//  XmlOps::print_xml_node( *doc.get() );
//  XmlOps::write_xml_node( *doc.get(),  "test.xml" );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( rename )
{
  CRoot::Ptr root = CRoot::create ( "Simulator" );

  Component::Ptr c1 = root->create_component_ptr<Component>("c1");

  BOOST_CHECK_EQUAL ( c1->name(), "c1" );

  Component::Ptr c2 = root->create_component_ptr<Component>("c1");

  BOOST_CHECK_EQUAL ( c2->name(), "c1_1" );

  Component::Ptr c3 = root->create_component_ptr<Component>("c1");

  BOOST_CHECK_EQUAL ( c3->name(), "c1_2" );

  // turn off exception dumping
  ExceptionManager::instance().ExceptionDumps = false;

  BOOST_CHECK_THROW ( c2->rename("c1_2"), ValueExists );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
