// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for XML options manipulation"

#include <iostream>

#include "rapidxml/rapidxml.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>

#include "Common/OptionArray.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"

#include "Common/XML/FileOperations.hpp"
#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalOptions.hpp"

using namespace boost::assign;
using namespace CF;
using namespace CF::Common;
using namespace CF::Common::XML;

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( XmlSignalOptions_TestSuite )

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( contructors )
{
  //
  // We create an XML frame with some options, build an OptionList with it
  // and check that options were correctly parsed.
  //

  SignalFrame frame;
  Map map = frame.map( Protocol::Tags::key_options() ).main_map;
  std::vector<std::string> data(4);

  data[0] = "This is the first item";
  data[1] = "Here is the second item";
  data[2] = "Now we have the third item";
  data[3] = "And then the last item";

  map.set_value<bool>("my_bool", false, "MyBool description").set_attribute( "mode", "adv" );
  map.set_value<CF::Real>("cfl", 3.1415, "CFL number").set_attribute( "mode", "basic" );
  map.set_array<std::string>("strings", data, ";", "Some special data"); // should be advanced by default
  map.set_value<URI>("website", URI("http://coolfluidsrv.vki.ac.be"), "CF website")
      .set_attribute( Protocol::Tags::attr_uri_schemes(), "http");

  SignalOptions options( frame );

  //
  // 1. check option "my_bool"
  //
  OptionT<bool>::Ptr my_bool;
  BOOST_CHECK( options.check("my_bool") );
  my_bool = boost::dynamic_pointer_cast<OptionT<bool> >(options.store["my_bool"]);
  BOOST_CHECK( my_bool.get() != nullptr );
  BOOST_CHECK_EQUAL( my_bool->name(), std::string("my_bool") );
  BOOST_CHECK_EQUAL( my_bool->type(), std::string( Protocol::Tags::type<bool>() ) );
  BOOST_CHECK_EQUAL( my_bool->description(), std::string("MyBool description") );
  BOOST_CHECK_EQUAL( my_bool->value<bool>(), false );
  BOOST_CHECK_EQUAL( my_bool->has_tag("basic"), false );


  //
  // 2. check option "cfl"
  //
  OptionT<CF::Real>::Ptr cfl;
  BOOST_CHECK( options.check("cfl") );
  cfl = boost::dynamic_pointer_cast<OptionT<CF::Real> >(options.store["cfl"]);
  BOOST_CHECK( cfl.get() != nullptr );
  BOOST_CHECK_EQUAL( cfl->name(), std::string("cfl") );
  BOOST_CHECK_EQUAL( cfl->type(), std::string( Protocol::Tags::type<CF::Real>() ) );
  BOOST_CHECK_EQUAL( cfl->description(), std::string("CFL number") );
  BOOST_CHECK_EQUAL( cfl->value<CF::Real>(), 3.1415 );
  BOOST_CHECK_EQUAL( cfl->has_tag("basic"), true );

  //
  // 3. check option "my_bool"
  //
  OptionArrayT<std::string>::Ptr strings;
  BOOST_CHECK( options.check("strings") );
  strings = boost::dynamic_pointer_cast<OptionArrayT<std::string> >(options.store["strings"]);
  BOOST_CHECK( strings.get() != nullptr );
  BOOST_CHECK_EQUAL( strings->name(), std::string("strings") );
  BOOST_CHECK_EQUAL( std::string(strings->elem_type()), std::string(Protocol::Tags::type<std::string>()) );
  BOOST_CHECK_EQUAL( strings->description(), std::string("Some special data") );
  BOOST_CHECK_EQUAL( strings->has_tag("basic"), false );

  std::vector<std::string> data_read = strings->value_vect();
  BOOST_CHECK_EQUAL( data.size(), data_read.size() );

  for(int i = 0 ; i < data.size() ; ++i)
    BOOST_CHECK_EQUAL( data[i], data_read[i] );

  //
  // 4. check option "website"
  //
  OptionURI::Ptr website;
  BOOST_CHECK( options.check("website") );
  website = boost::dynamic_pointer_cast<OptionURI>(options.store["website"]);
  BOOST_CHECK( website.get() != nullptr );
  BOOST_CHECK_EQUAL( website->name(), std::string("website") );
  BOOST_CHECK_EQUAL( website->type(), std::string( Protocol::Tags::type<URI>() ) );
  BOOST_CHECK_EQUAL( website->description(), std::string("CF website") );
  BOOST_CHECK_EQUAL( website->value<URI>().string(), std::string("http://coolfluidsrv.vki.ac.be") );
  BOOST_CHECK_EQUAL( website->has_tag("basic"), false );
  BOOST_CHECK_EQUAL( website->supported_protocols().size(), 1 );
  BOOST_CHECK_EQUAL( website->supported_protocols()[0], URI::Scheme::HTTP );
}

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( xml_to_option )
{
  XmlDoc::Ptr xmldoc;
  Option::Ptr option;
  XmlNode node;

  //
  // 1. a Real option with a description
  //
  xmldoc = parse_cstring(
      "<value key=\"theOption\" is_option=\"true\" descr=\"This is a description\">"
      "  <real>2.71</real>"
      "</value>");
  node.content = xmldoc->content->first_node();
  BOOST_CHECK_NO_THROW( option = SignalOptions::xml_to_option(node) );
  // 1a. check the name
  BOOST_CHECK_EQUAL( option->name(), std::string("theOption"));
  // 1b. check the type
  BOOST_CHECK_EQUAL( option->type(), std::string("real") );
  // 1c. check the value
  BOOST_CHECK_EQUAL( option->value<Real>(), Real(2.71) );
  // 1d. check the description
  BOOST_CHECK_EQUAL( option->description(), std::string("This is a description") );

  //
  // 2. an Uint option with a missing description
  //
  xmldoc = parse_cstring(
      "<value key=\"theAnswer\" is_option=\"true\">"
      "  <integer>42</integer>"
      "</value>");
  node.content = xmldoc->content->first_node();
  BOOST_CHECK_NO_THROW( option = SignalOptions::xml_to_option(node) );
  // 2a. check the name
  BOOST_CHECK_EQUAL( option->name(), std::string("theAnswer"));
  // 2b. check the type
  BOOST_CHECK_EQUAL( option->type(), std::string("integer") );
  // 2c. check the value
  BOOST_CHECK_EQUAL( option->value<int>(), int(42) );
  // 2d. check the description
  BOOST_CHECK_EQUAL( option->description(), std::string("") );

  //
  // 3. no key attribute
  //
  xmldoc = XML::parse_cstring(
      "<value is_option=\"true\">"
      "  <integer>42</integer>"
      "</value>");
  node.content = xmldoc->content->first_node();
  BOOST_CHECK_THROW( SignalOptions::xml_to_option(node), ProtocolError );

  //
  // 4. option mode (basic/adv)
  //
  XmlDoc doc;
  Map map( doc.add_node( Protocol::Tags::node_map() ) );

  XmlNode theAnswer = map.set_value("theAnswer", int(42) );
  XmlNode pi = map.set_value("pi", Real(3.14159) );
  XmlNode euler = map.set_value("euler", Real(2.71) );

  // theAnswer is not marked as basic
  pi.set_attribute("mode", "basic");
  euler.set_attribute("mode", "adv");

  BOOST_CHECK( !SignalOptions::xml_to_option(theAnswer)->has_tag("basic") );
  BOOST_CHECK( SignalOptions::xml_to_option(pi)->has_tag("basic") );
  BOOST_CHECK( !SignalOptions::xml_to_option(euler)->has_tag("basic") );
}

//////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( xml_to_option_types )
{
  XmlDoc::Ptr xmldoc(new XmlDoc());
  Option::Ptr option;

  Map map(xmldoc->add_node("map"));

  XmlNode optBool = map.set_value("optBool", true);
  XmlNode optInt = map.set_value("optInt", int(-15468));
  XmlNode optUint = map.set_value("optUint", Uint(17513214));
  XmlNode optReal = map.set_value("optReal", Real(3.14159));
  XmlNode optString = map.set_value("optString", std::string("I am a string value"));
  XmlNode optURI = map.set_value("optURI", URI("cpath://Root"));

  XmlNode wrongOpt = map.content.add_node( Protocol::Tags::node_value() );

  wrongOpt.set_attribute( Protocol::Tags::attr_key(), "optOfUnknownType");
  wrongOpt.add_node("unknown_type", "Don't know how to interpret this.");

  // 1. bool
  BOOST_CHECK_NO_THROW( option = SignalOptions::xml_to_option(optBool) );
  BOOST_CHECK_EQUAL( option->type(), std::string("bool") );
  BOOST_CHECK_EQUAL( option->value<bool>(), true);

  // 2. int
  BOOST_CHECK_NO_THROW( option = SignalOptions::xml_to_option(optInt) );
  BOOST_CHECK_EQUAL( option->type(), std::string("integer") );
  BOOST_CHECK_EQUAL( option->value<int>(), -15468 );

  // 3. Uint
  BOOST_CHECK_NO_THROW( option = SignalOptions::xml_to_option(optUint) );
  BOOST_CHECK_EQUAL( option->type(), std::string("unsigned") );
  BOOST_CHECK_EQUAL( option->value<Uint>(), Uint(17513214) );

  // 4. Real
  BOOST_CHECK_NO_THROW( option = SignalOptions::xml_to_option(optReal) );
  BOOST_CHECK_EQUAL( option->type(), std::string("real") );
  BOOST_CHECK_EQUAL( option->value<Real>(), Real(3.14159) );

  // 5. string
  BOOST_CHECK_NO_THROW( option = SignalOptions::xml_to_option(optString) );
  BOOST_CHECK_EQUAL( option->type(), std::string("string") );
  BOOST_CHECK_EQUAL( option->value<std::string>(), std::string("I am a string value") );

  // 6. uri
  BOOST_CHECK_NO_THROW( option = SignalOptions::xml_to_option(optURI) );
  BOOST_CHECK_EQUAL( option->type(), std::string("uri") );
  BOOST_CHECK_EQUAL( option->value<URI>().string(), std::string("cpath://Root") );

  // 7. unknown type
  BOOST_CHECK_THROW( SignalOptions::xml_to_option(wrongOpt), ShouldNotBeHere );
}

//////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( xml_to_option_uri_schemes )
{
  XmlDoc::Ptr xmldoc(new XmlDoc());
  Map map(xmldoc->add_node("map"));
  const char * tag = Protocol::Tags::attr_uri_schemes();

  XmlNode opt1 = map.set_value("opt1", URI());
  XmlNode opt2 = map.set_value("opt2", URI());
  XmlNode opt3 = map.set_value("opt3", URI());
  XmlNode opt4 = map.set_value("opt4", URI());
  XmlNode opt5 = map.set_value("opt5", URI());

  // opt1 has no scheme attribute defined
  opt2.set_attribute(tag, "");                  // no scheme defined
  opt3.set_attribute(tag, "http");              // one scheme
  opt4.set_attribute(tag, "cpath,https,file");  // several schemes
  opt5.set_attribute(tag, "cpath,scheme");      // a wrong scheme

  OptionURI::Ptr opt;
  std::vector<URI::Scheme::Type> vect;

  // 1. check opt1
  opt = boost::dynamic_pointer_cast<OptionURI>( SignalOptions::xml_to_option(opt1) );

  BOOST_CHECK( opt->supported_protocols().empty() );

  // 2. check opt2
  opt = boost::dynamic_pointer_cast<OptionURI>( SignalOptions::xml_to_option(opt2) );

  BOOST_CHECK( opt->supported_protocols().empty() );

  // 3. check opt3
  opt = boost::dynamic_pointer_cast<OptionURI>( SignalOptions::xml_to_option(opt3) );

  vect = opt->supported_protocols();

  BOOST_CHECK_EQUAL( vect.size() , size_t(1) );
  BOOST_CHECK_EQUAL( vect[0], URI::Scheme::HTTP );

  // 4. check opt4
  opt = boost::dynamic_pointer_cast<OptionURI>( SignalOptions::xml_to_option(opt4) );

  vect = opt->supported_protocols();

  BOOST_CHECK_EQUAL( vect.size() , size_t(3) );
  BOOST_CHECK_EQUAL( vect[0], URI::Scheme::CPATH );
  BOOST_CHECK_EQUAL( vect[1], URI::Scheme::HTTPS );
  BOOST_CHECK_EQUAL( vect[2], URI::Scheme::FILE  );

  // 5. check opt5
  BOOST_CHECK_THROW( SignalOptions::xml_to_option( opt5 ), CastingFailed);
}

//////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( xml_to_option_restricted_lists )
{
  XmlDoc::Ptr xmldoc(new XmlDoc());
  Map map(xmldoc->add_node("map"));
  Option::Ptr option;
  std::vector<int> vectInt = list_of<int>(344646)(544684)(446454)
                                                        (878764)(646316);

  XmlNode optInt = map.set_value("optInt", int(13));
  XmlNode optIntRestrList = map.set_value("optIntRestrList", int(-15468));

  Map(optIntRestrList).set_array(Protocol::Tags::key_restricted_values(), vectInt, " ; ");

  // test without restricted list
  BOOST_CHECK_NO_THROW(option = SignalOptions::xml_to_option( optInt ) );
  BOOST_CHECK( !option->has_restricted_list() );

  // test with restricted list
  BOOST_CHECK_NO_THROW(option = SignalOptions::xml_to_option( optIntRestrList ) );

  BOOST_CHECK( option->has_restricted_list() );

  std::vector<boost::any> & restr_list = option->restricted_list();

  BOOST_CHECK_EQUAL( restr_list.size(), size_t( vectInt.size() + 1 ) );
  BOOST_CHECK_EQUAL( boost::any_cast<int>(restr_list[0]), int(-15468) );
  BOOST_CHECK_EQUAL( boost::any_cast<int>(restr_list[1]), vectInt[0] );
  BOOST_CHECK_EQUAL( boost::any_cast<int>(restr_list[2]), vectInt[1] );
  BOOST_CHECK_EQUAL( boost::any_cast<int>(restr_list[3]), vectInt[2] );
  BOOST_CHECK_EQUAL( boost::any_cast<int>(restr_list[4]), vectInt[3] );
  BOOST_CHECK_EQUAL( boost::any_cast<int>(restr_list[5]), vectInt[4] );
}

//////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

/////////////////////////////////////////////////////////////////////////////
