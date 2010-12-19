// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/foreach.hpp>
#include <boost/iterator.hpp>

#include "Common/Log.hpp"
#include "Common/Component.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/CRoot.hpp"
#include "Common/CGroup.hpp"
#include "Common/CLink.hpp"
#include "Common/XmlHelpers.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( Component_TestSuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructors )
{
  // constructor with passed path
  CRoot::Ptr root = CRoot::create ( "root" );

  BOOST_CHECK_EQUAL ( root->name() , "root" );
  BOOST_CHECK_EQUAL ( root->path().string() , "/" );
  BOOST_CHECK_EQUAL ( root->full_path().string() , "//root" );

  BOOST_CHECK_EQUAL ( root->properties().check("brief") , true );
  BOOST_CHECK_EQUAL ( root->properties().check("description") , true );

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
  CRoot::Ptr root = CRoot::create ( "root" );

  Component::Ptr dir1 ( new CGroup ( "dir1" ) );
  Component::Ptr dir2 ( new CGroup ( "dir2" ) );

  root->add_component( dir1 );
  dir1->add_component( dir2 );

  BOOST_CHECK_EQUAL ( root->full_path().string() , "//root" );
  BOOST_CHECK_EQUAL ( dir1->full_path().string() , "//root/dir1" );
  BOOST_CHECK_EQUAL ( dir2->full_path().string() , "//root/dir1/dir2" );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( is_link )
{
  CRoot::Ptr root = CRoot::create ( "root" );

  Component::Ptr dir1 ( new CGroup ( "dir1" ) );

  root->add_component( dir1 );

  BOOST_CHECK ( ! root->is_link() );
  BOOST_CHECK ( ! dir1->is_link() );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( get )
{
  CRoot::Ptr root = CRoot::create ( "root" );

  Component::Ptr dir1 ( new CGroup ( "dir1" ) );
  Component::Ptr lnk1 ( new CLink  ( "lnk1" ) );

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
  CRoot::Ptr root = CRoot::create ( "root" );

  Component::Ptr dir1 ( new CGroup ( "dir1" ) );
  Component::Ptr dir2 ( new CGroup ( "dir2" ) );
  Component::Ptr dir3 ( new CGroup ( "dir3" ) );

  // add child components to root
  root->add_component( dir1 );
  dir1->add_component( dir2 );
  dir2->add_component( dir3 );

  // test absolute & complete path
  URI p0 ( "cpath://root/dir1" );
  dir2->complete_path( p0 );
  BOOST_CHECK_EQUAL ( p0.string(), "//root/dir1" );

  // test relative
  URI p10 ( "cpath:.." );
  dir2->complete_path( p10 );
  BOOST_CHECK_EQUAL ( p10.string(), "//root/dir1" );

  // test relative
  URI p11 ( "cpath:./" );
  dir2->complete_path( p11 );
  BOOST_CHECK_EQUAL ( p11.string(), "//root/dir1/dir2" );

  // test relative & complete path
  URI p12 ( "cpath:../../dir2" );
  dir3->complete_path( p12 );
  BOOST_CHECK_EQUAL ( p12.string(), "//root/dir1/dir2" );

  // test absolute & incomplete path
  URI p2 ( "cpath://root/dir1/dir2/../dir2" );
  dir2->complete_path( p2 );
  BOOST_CHECK_EQUAL ( p2.string(), "//root/dir1/dir2" );

  // test absolute & multiple incomplete path
  URI p3 ( "cpath://root/dir1/../dir2/../dir1/../dir2/dir3" );
  dir2->complete_path( p3 );
  BOOST_CHECK_EQUAL ( p3.string(), "//root/dir2/dir3" );

  // test absolute & multiple incomplete path at end
  URI p4 ( "cpath://root/dir1/dir2/dir3/../../" );
  dir2->complete_path( p4 );
  BOOST_CHECK_EQUAL ( p4.string(), "//root/dir1" );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( look_component )
{
  CRoot::Ptr root = CRoot::create ( "root" );

  Component::Ptr dir1  ( new CGroup ( "dir1" ) );
  Component::Ptr dir2  ( new CGroup ( "dir2" ) );
  Component::Ptr dir21 ( new CGroup ( "dir21" ) );
  Component::Ptr dir22 ( new CGroup ( "dir22" ) );

  // add child components to root
  root->add_component( dir1 );
  dir1->add_component( dir2 );
  dir2->add_component( dir21 );
  dir2->add_component( dir22 );

  // test relative & complete path
  URI p0 ( "cpath:../dir21" );
  Component::Ptr cp0 = dir22->look_component( p0 );
  BOOST_CHECK_EQUAL ( cp0->full_path().string(), "//root/dir1/dir2/dir21" );

  // test relative & complete path
  URI p1 ( "cpath://root/dir1" );
  Component::Ptr cp1 = dir22->look_component( p1 );
  BOOST_CHECK_EQUAL ( cp1->full_path().string(), "//root/dir1" );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( change_parent )
{
  CRoot::Ptr root = CRoot::create ( "root" );

  Component::Ptr dir1  ( new CGroup ( "dir1" ) );
  Component::Ptr dir2  ( new CGroup ( "dir2" ) );

  // add child components to root
  root->add_component( dir1 );
  dir1->add_component( dir2 );

  BOOST_CHECK_EQUAL ( dir2->full_path().string(), "//root/dir1/dir2" );

  dir2->change_parent( root.get() );

  BOOST_CHECK_EQUAL ( dir2->full_path().string(), "//root/dir2" );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( problem )
{
  CRoot::Ptr root = CRoot::create ( "Simulator" );

  Component::Ptr proot = root->look_component("cpath://Simulator");

  BOOST_CHECK_EQUAL ( proot->full_path().string(), "//Simulator" );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( create_subcomponents )
{
  CRoot::Ptr root = CRoot::create ( "root" );
  Component::Ptr comp1 = root->create_component<Component>("comp1");
  comp1->create_component<Component>("comp1_1");
  comp1->create_component<Component>("comp1_2");

  BOOST_CHECK_EQUAL(find_component_with_name(*root, "comp1").name(),"comp1");
  BOOST_CHECK_EQUAL(find_component_recursively_with_name(*root, "comp1_1").name(),"comp1_1");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( create_component_signal )
{
  CRoot::Ptr root = CRoot::create ( "croot" );

  boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc ();


  XmlNode& node = *XmlOps::goto_doc_node( *doc.get() );

  XmlParams params ( node );

  params.add_option<std::string>( "name",  "MyMesh" );
  params.add_option<std::string>( "atype", "CMeshReader" );
  params.add_option<std::string>( "ctype", "CGNS" );

//  XmlOps::print_xml_node( *doc.get() );
//  XmlOps::write_xml_node( *doc.get(),  "test.xml" );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

