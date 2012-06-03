// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for XML maps manipulation"

#include <boost/assign/list_of.hpp>

#include <boost/test/unit_test.hpp>

#include "rapidxml/rapidxml.hpp"

#include "common/CF.hpp"
#include "common/BasicExceptions.hpp"
#include "common/StringConversion.hpp"

#include "common/XML/Protocol.hpp"

#include "common/XML/Map.hpp"
#include <common/TypeInfo.hpp>
#include <common/Option.hpp>
#include <common/Core.hpp>
#include <common/Environment.hpp>
#include <common/OptionList.hpp>

using namespace cf3;
using namespace cf3::common;
using namespace cf3::common::XML;
using namespace boost::assign; // for list_of

/////////////////////////////////////////////////////////////////////////////

struct XmlFixture
{
  XmlFixture()
  {
    Core::instance().environment().options().set("exception_backtrace", false);
    Core::instance().environment().options().set("exception_outputs", false);
  }
};

BOOST_FIXTURE_TEST_SUITE( XmlMap_TestSuite, XmlFixture )

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( set_value )
{
  XmlNode node(new rapidxml::xml_document<>());
  Map map(node);
  XmlNode value_node;
  XmlNode mod_value_node;
  XmlNode type_node;

  rapidxml::xml_attribute<>* key_attr = nullptr;

  //
  // 1. the key is empty
  BOOST_CHECK_THROW ( map.set_value( "", common::class_name<bool>(), "true" ), BadValue );

  //
  // 2. the value does not exist, setting it should create it and the
  // correct node should be returned
  //

  value_node = map.set_value( "TheUltimateAnswer", common::class_name<int>(), "12" );
  BOOST_CHECK ( value_node.is_valid() );

  // 2a. it should be a "value" node
  BOOST_CHECK_EQUAL ( std::strcmp(value_node.content->name(), Protocol::Tags::node_value()), 0 );

  // 2b. it should have the right key value
  key_attr = value_node.content->first_attribute( Protocol::Tags::attr_key() );
  BOOST_CHECK ( is_not_null(key_attr) );
  BOOST_CHECK_EQUAL ( std::strcmp(key_attr->value(), "TheUltimateAnswer"), 0 );

  // 2c. it should have the right value with the right type
  type_node = XmlNode( value_node.content->first_node( common::class_name<int>().c_str() ) );
  BOOST_CHECK ( type_node.is_valid() );
  BOOST_CHECK_EQUAL ( std::strcmp(type_node.content->value(), "12"), 0 );

  //
  // 3. test the type checking
  //

  // 3a. try to change the value but with a wrong type
  BOOST_CHECK_THROW ( map.set_value("TheUltimateAnswer", common::class_name<std::string>(), std::string()), XmlError );

  // 3b. the value in the xml tree has no type
  node.add_node( Protocol::Tags::node_value() ).set_attribute( Protocol::Tags::attr_key(), "hello");
  BOOST_CHECK_THROW ( map.set_value("hello", common::class_name<std::string>(),std::string()), XmlError );

  // 3c. the value in the xml tree is an array
  node.add_node( Protocol::Tags::node_array() ).set_attribute( Protocol::Tags::attr_key(), "world");
  BOOST_CHECK_THROW ( map.set_value("world", common::class_name<std::string>(), std::string()), XmlError );

  //
  // 4. change the value
  //

  mod_value_node = map.set_value( "TheUltimateAnswer", common::class_name<int>(), "42" );
  BOOST_CHECK ( mod_value_node.is_valid() );

  // 4a. it should be the same node as the one before (same object/pointer)
  BOOST_CHECK_EQUAL ( mod_value_node.content, value_node.content );

  // 4b. it should the right value with the right type
  type_node = XmlNode( mod_value_node.content->first_node( common::class_name<int>().c_str() ) );
  BOOST_CHECK ( type_node.is_valid() );
  BOOST_CHECK_EQUAL ( std::strcmp(type_node.content->value(), "42"), 0 );

  // clear the memory pool
  delete node.content->document();
}

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( set_array )
{
  XmlNode node(new rapidxml::xml_document<>());
  Map map(node);
  XmlNode value_node;
  XmlNode mod_value_node;

  std::vector<int> vect_first = list_of<int>(12)(5)(546)(2135)(12164)(3464)(1);
  std::vector<int> vect_second = list_of<int>(7987)(346)(101);
  cf3::Uint size_first = vect_first.size();
  cf3::Uint size_second = vect_second.size();

  // the values as they should be written in the array
  std::string str_first("12 ; 5 ; 546 ; 2135 ; 12164 ; 3464 ; 1"); // delimiter is " ; "
  std::string str_second("7987_|_346_|_101"); // delimiter is "_|_"

  rapidxml::xml_attribute<>* tmp_attr = nullptr;


  //
  // 1. test the argument validity checking
  //

  // 1b. the key is empty
  BOOST_CHECK_THROW ( map.set_array( "", common::class_name<int>(), common::option_vector_to_str(vect_first, " ; "), " ; "), BadValue );

  // 1c. the delimiter is empty
  BOOST_CHECK_THROW ( map.set_array( "Array", common::class_name<int>(), common::option_vector_to_str(vect_first, ""), ""), BadValue );

  //
  // 2. the value does not exist, setting it should create it and the
  // correct node should be returned
  //

  value_node = map.set_array( "TheArray", common::class_name<int>(), option_vector_to_str(vect_first, " ; "), " ; " );
  BOOST_CHECK ( value_node.is_valid() );

  // 2a. it should be a "array" node
  BOOST_CHECK_EQUAL ( std::strcmp(value_node.content->name(), Protocol::Tags::node_array()), 0 );

  // 2b. it should have the right key value
  tmp_attr = value_node.content->first_attribute( Protocol::Tags::attr_key() );
  BOOST_CHECK ( is_not_null(tmp_attr) );
  BOOST_CHECK_EQUAL ( std::string(tmp_attr->value()), std::string("TheArray") );

  // 2c. it should have the right type
  tmp_attr = value_node.content->first_attribute( Protocol::Tags::attr_array_type() );
  BOOST_CHECK ( is_not_null(tmp_attr) );
  BOOST_CHECK_EQUAL ( std::string(tmp_attr->value()), std::string(common::class_name<int>() ));

  // 2d. it should have the right size
  tmp_attr = value_node.content->first_attribute( Protocol::Tags::attr_array_size() );
  BOOST_CHECK ( is_not_null(tmp_attr) );
  BOOST_CHECK_EQUAL ( std::string(tmp_attr->value()), std::string( to_str(size_first) ));

  // 2e. it should have the right delimiter
  tmp_attr = value_node.content->first_attribute( Protocol::Tags::attr_array_delimiter() );
  BOOST_CHECK ( is_not_null(tmp_attr) );
  BOOST_CHECK_EQUAL ( std::string(tmp_attr->value()), std::string(" ; ") );

  // 2f. it should have the right value
  BOOST_CHECK ( is_not_null(value_node.content->value()) );
  BOOST_CHECK_EQUAL ( std::string(value_node.content->value()), str_first );

  //
  // 3. test the type checking
  //

  // 3a. try to change the array but with a wrong type
  std::vector<std::string> vect_wrong;
  BOOST_CHECK_THROW ( map.set_array( "TheArray", common::class_name<std::string>(),  common::option_vector_to_str(vect_wrong, " ; "), " ; " ), XmlError );

  // 3b. the array in the xml tree has no type
  node.add_node( Protocol::Tags::node_array() ).set_attribute( Protocol::Tags::attr_key(), "hello");
  BOOST_CHECK_THROW ( map.set_array( "hello", common::class_name<std::string>(),  common::option_vector_to_str(vect_wrong, " ; "), " ; " ), XmlError );

  // 3c. the arry in the xml tree is a single value
  node.add_node( Protocol::Tags::node_value() ).set_attribute( Protocol::Tags::attr_key(), "world");
  BOOST_CHECK_THROW ( map.set_array( "world", common::class_name<std::string>(),  common::option_vector_to_str(vect_wrong, " ; "), " ; " ), XmlError );

  //
  // 4. change the value
  //

  mod_value_node = map.set_array( "TheArray", common::class_name<int>(),  common::option_vector_to_str(vect_second, "_|_"), "_|_" );
  BOOST_CHECK ( mod_value_node.is_valid() );

  // 4a. it should be the same node as the one before (same object/pointer)
  BOOST_CHECK_EQUAL ( mod_value_node.content, value_node.content );

  // 4b. the size should have been updated
  tmp_attr = mod_value_node.content->first_attribute( Protocol::Tags::attr_array_size() );
  BOOST_CHECK ( is_not_null(tmp_attr) );
  BOOST_CHECK_EQUAL ( std::string(tmp_attr->value()), std::string( to_str(size_second) ));

  // 4c. the delimiter should have been updated
  tmp_attr = mod_value_node.content->first_attribute( Protocol::Tags::attr_array_delimiter() );
  BOOST_CHECK ( is_not_null(tmp_attr) );
  BOOST_CHECK_EQUAL ( std::string(tmp_attr->value()), std::string("_|_") );

  // 4d. the value should have been updated
  BOOST_CHECK ( is_not_null(mod_value_node.content->value()) );
  BOOST_CHECK_EQUAL ( std::string(mod_value_node.content->value()), str_second );

  // clear the memory pool
  delete node.content->document();
}

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( find_value )
{
  XmlNode node(new rapidxml::xml_document<>());
  Map map(node);
  XmlNode first_node( node.add_node(Protocol::Tags::node_map()) );
  XmlNode second_node( node.add_node(Protocol::Tags::node_value()) );
  XmlNode found_node;
  rapidxml::xml_attribute<>* key_attr = nullptr;

  first_node.set_attribute( Protocol::Tags::attr_key(), "FirstNode" );
  second_node.set_attribute( Protocol::Tags::attr_key(), "SecondNode" );

  //
  // 1. seeking a node with empty key should return FirstNode (the first node found)
  //

  found_node = map.find_value();

  BOOST_CHECK ( found_node.is_valid() );
  BOOST_CHECK_EQUAL ( found_node.content, first_node.content );

  //
  // 2. seeking a node with non-empty key should return the correct node
  //

  found_node = map.find_value( "SecondNode" );

  BOOST_CHECK ( found_node.is_valid() );
  BOOST_CHECK_EQUAL ( found_node.content, second_node.content );

  //
  // 3. if the node is not found, an invalid node should be returned
  //

  found_node = map.find_value( "MapThatDoesNotExist" );

  BOOST_CHECK ( !found_node.is_valid() );

  // clear the memory pool
  delete node.content->document();
}

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( is_single_value )
{
  XmlNode node;
  XmlNode added_node;

  // 1. the node is not valid, should return false
  BOOST_CHECK ( !Map::is_single_value(node) );

  node.content = new rapidxml::xml_document<>();

  // 2. the node is an array value, should return false
  added_node = node.add_node( Protocol::Tags::node_array() );
  BOOST_CHECK ( !Map::is_single_value(added_node) );

  // 3. the node is a single value, should return true
  added_node = node.add_node( Protocol::Tags::node_value() );
  BOOST_CHECK ( Map::is_single_value(added_node) );

  // clear the memory pool
  delete node.content->document();
}

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( is_array_value )
{
  XmlNode node;
  XmlNode added_node;

  // 1. the node is not valid, should return false
  BOOST_CHECK ( !Map::is_array_value(node) );

  node.content = new rapidxml::xml_document<>();

  // 2. the node is a single value, should return false
  added_node = node.add_node( Protocol::Tags::node_value() );
  BOOST_CHECK ( !Map::is_array_value(added_node) );

  // 3. the node is an array value, should return true
  added_node = node.add_node( Protocol::Tags::node_array() );
  BOOST_CHECK ( Map::is_array_value(added_node) );

  // clear the memory pool
  delete node.content->document();
}

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( get_value_type )
{
  XmlNode node(new rapidxml::xml_document<>());
  Map map(node);
  std::vector<int> vect;

  XmlNode value( map.set_value("MyString", class_name<std::string>(), std::string("TheString")) );
  XmlNode array( map.set_array("MyVector", class_name<int>(), "", " ; ") );

  XmlNode wrong_value( node.add_node(Protocol::Tags::node_value()) );
  XmlNode wrong_array( node.add_node(Protocol::Tags::node_array()) );
  XmlNode wrong_node( node.add_node(Protocol::Tags::node_map()) );

  wrong_value.set_attribute( Protocol::Tags::attr_key(), "AValue" );
  wrong_array.set_attribute( Protocol::Tags::attr_key(), "AnArray" );
  wrong_node.set_attribute( Protocol::Tags::attr_key(), "AMap");

  // the node is correct and the right type should be returned
  BOOST_CHECK_EQUAL ( std::strcmp( Map::get_value_type(value), common::class_name<std::string>().c_str() ), 0);
  BOOST_CHECK_EQUAL ( std::strcmp( Map::get_value_type(array), common::class_name<int>().c_str() ), 0);

  // the type is wrong, an exception should be thrown
  BOOST_CHECK_THROW ( Map::get_value_type(wrong_value), XmlError);
  BOOST_CHECK_THROW ( Map::get_value_type(wrong_array), XmlError);
  BOOST_CHECK_THROW ( Map::get_value_type(wrong_node), XmlError);

  // clear the memory pool
  delete node.content->document();
}

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( get_value )
{
  XmlNode node(new rapidxml::xml_document<>());
  Map map(node);
  XmlNode added_node;
  std::vector<int> int_vals = list_of<int>(1213)(5464)(5554)(5654)(273)(554)(354);
  std::string str("Hello, World!");

  map.set_value( "AString", class_name<std::string>(), str );
  map.set_value( "Zero", class_name<int>(), "0" );
  map.set_array( "SomeInts", class_name<int>(), option_vector_to_str(int_vals, " ; "), " ; " );

  // 1. try to get with a wrong type
  BOOST_CHECK_THROW ( map.get_value<int>( "AString"), XmlError );

  // 2. try to get an array (type is not important here)
  BOOST_CHECK_THROW ( map.get_value<int>( "SomeInts"), XmlError );

  // 3. get the value with the correct type
  BOOST_CHECK_EQUAL ( map.get_value<std::string>( "AString"), str );

  // clear the memory pool
  delete node.content->document();
}

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( get_array )
{
  XmlNode node(new rapidxml::xml_document<>());
  Map map(node);
  XmlNode added_node;
  std::vector<int> int_vals = list_of<int>(1213)(5464)(5554)(5654)(273)(554)(354);
  std::vector<int> int_read;
  std::vector<int>::iterator it_vals = int_vals.begin();
  std::vector<int>::iterator it_read;
  std::string str("Hello, World!");

  map.set_value( "AString", class_name<std::string>(), str );
  map.set_value( "Zero", class_name<int>(), "0" );
  map.set_array( "SomeInts", class_name<int>(), option_vector_to_str(int_vals, " ; "), " ; " );

  // 1. try to get with a wrong type
  BOOST_CHECK_THROW ( map.get_array<Real>( "SomeInts"), XmlError );

  // 2. try to get a signal value (type is not important here)
  BOOST_CHECK_THROW ( map.get_array<int>( "Zero"), XmlError );

  // 3. get the value with the correct type
  BOOST_CHECK_NO_THROW ( int_read = map.get_array<int>( "SomeInts") );

  BOOST_CHECK_EQUAL ( int_read.size(), int_vals.size() );

  // check that items match
  for( it_read = int_read.begin() ; it_read != int_read.end() ; ++it_read, ++it_vals )
    BOOST_CHECK_EQUAL ( *it_read, *it_vals);

  // clear the memory pool
  delete node.content->document();
}

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( split_string )
{
  XmlNode node(new rapidxml::xml_document<>());
  Map map(node);
  XmlNode added_node;
  std::vector<std::string> str_vals = list_of<std::string>("hello")("hello with some white spaces");
  std::vector<std::string> str_read;
  std::vector<std::string>::iterator it_vals = str_vals.begin();
  std::vector<std::string>::iterator it_read;

  map.set_array( "SomeStrings", class_name<std::string>(), option_vector_to_str(str_vals, " ; "), " ; " );

  // get the value
  BOOST_CHECK_NO_THROW ( str_read = map.get_array<std::string>( "SomeStrings") );

  // check that sizes match
  BOOST_CHECK_EQUAL ( str_read.size(), str_vals.size() );

  // check that items match
  for( it_read = str_read.begin() ; it_read != str_read.end() ; ++it_read, ++it_vals )
    BOOST_CHECK_EQUAL ( *it_read, *it_vals);

  // clear the memory pool
  delete node.content->document();
}

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

/////////////////////////////////////////////////////////////////////////////
