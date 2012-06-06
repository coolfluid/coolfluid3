// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

#include "common/OptionArray.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include <common/Core.hpp>
#include <common/Environment.hpp>
#include <common/OptionList.hpp>

#include "common/XML/FileOperations.hpp"
#include "common/XML/Protocol.hpp"
#include "common/XML/SignalFrame.hpp"
#include "common/XML/SignalOptions.hpp"

using namespace boost::assign;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::common::XML;

/////////////////////////////////////////////////////////////////////////////

struct XmlFixture
{
  XmlFixture()
  {
    Core::instance().environment().options().set("exception_backtrace", false);
    Core::instance().environment().options().set("exception_outputs", false);
  }
};

BOOST_FIXTURE_TEST_SUITE( XmlSignalOptions_TestSuite, XmlFixture )

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

  map.set_value("my_bool", class_name<bool>(), "false", "MyBool description").set_attribute( "mode", "adv" );
  map.set_value("cfl", class_name<Real>(), "3.1415", "CFL number").set_attribute( "mode", "basic" );
  map.set_array("strings", class_name<std::string>(), option_vector_to_str(data, ";"), ";", "Some special data"); // should be advanced by default
  map.set_value("website", class_name<URI>(), "http://coolfluidsrv.vki.ac.be", "CF website")
      .set_attribute( Protocol::Tags::attr_uri_schemes(), "http");

  SignalOptions options( frame );

  //
  // 1. check option "my_bool"
  //
  boost::shared_ptr< OptionT<bool> > my_bool;
  BOOST_CHECK( options.check("my_bool") );
  my_bool = boost::dynamic_pointer_cast<OptionT<bool> >(options.store["my_bool"]);
  BOOST_CHECK( my_bool.get() != nullptr );
  BOOST_CHECK_EQUAL( my_bool->name(), std::string("my_bool") );
  BOOST_CHECK_EQUAL( my_bool->type(), std::string( common::class_name<bool>() ) );
  BOOST_CHECK_EQUAL( my_bool->description(), std::string("MyBool description") );
  BOOST_CHECK_EQUAL( my_bool->value<bool>(), false );
  BOOST_CHECK_EQUAL( my_bool->has_tag("basic"), false );


  //
  // 2. check option "cfl"
  //
  boost::shared_ptr< OptionT<cf3::Real> > cfl;
  BOOST_CHECK( options.check("cfl") );
  cfl = boost::dynamic_pointer_cast<OptionT<cf3::Real> >(options.store["cfl"]);
  BOOST_CHECK( cfl.get() != nullptr );
  BOOST_CHECK_EQUAL( cfl->name(), std::string("cfl") );
  BOOST_CHECK_EQUAL( cfl->type(), std::string( common::class_name<cf3::Real>() ) );
  BOOST_CHECK_EQUAL( cfl->description(), std::string("CFL number") );
  BOOST_CHECK_EQUAL( cfl->value<cf3::Real>(), 3.1415 );
  BOOST_CHECK_EQUAL( cfl->has_tag("basic"), true );

  //
  // 3. check option "my_bool"
  //
  boost::shared_ptr< OptionArray<std::string> > strings;
  BOOST_CHECK( options.check("strings") );
  strings = boost::dynamic_pointer_cast<OptionArray<std::string> >(options.store["strings"]);
  BOOST_CHECK( strings.get() != nullptr );
  BOOST_CHECK_EQUAL( strings->name(), std::string("strings") );
  BOOST_CHECK_EQUAL( std::string(strings->element_type()), std::string(common::class_name<std::string>()) );
  BOOST_CHECK_EQUAL( strings->description(), std::string("Some special data") );
  BOOST_CHECK_EQUAL( strings->has_tag("basic"), false );

  std::vector<std::string> data_read = strings->value< std::vector<std::string> >();
  BOOST_CHECK_EQUAL( data.size(), data_read.size() );

  for(int i = 0 ; i < data.size() ; ++i)
    BOOST_CHECK_EQUAL( data[i], data_read[i] );

  //
  // 4. check option "website"
  //
  boost::shared_ptr< OptionURI > website;
  BOOST_CHECK( options.check("website") );
  website = boost::dynamic_pointer_cast<OptionURI>(options.store["website"]);
  BOOST_CHECK( website.get() != nullptr );
  BOOST_CHECK_EQUAL( website->name(), std::string("website") );
  BOOST_CHECK_EQUAL( website->type(), std::string( common::class_name<URI>() ) );
  BOOST_CHECK_EQUAL( website->description(), std::string("CF website") );
  BOOST_CHECK_EQUAL( website->value<URI>().string(), std::string("http://coolfluidsrv.vki.ac.be") );
  BOOST_CHECK_EQUAL( website->has_tag("basic"), false );
  BOOST_CHECK_EQUAL( website->supported_protocols().size(), 1 );
  BOOST_CHECK_EQUAL( website->supported_protocols()[0], URI::Scheme::HTTP );
}

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( xml_to_option )
{
  boost::shared_ptr< XmlDoc > xmldoc;
  boost::shared_ptr<Option> option;
  XmlNode node;

  //
  // 1. a Real option with a description
  //
  xmldoc = parse_cstring(
      "<value key=\"theOption\" is_option=\"true\" descr=\"This is a description\">"
      "  <real>2.71</real>"
      "</value>");
  node.content = xmldoc->content->first_node("value");
  BOOST_REQUIRE_NO_THROW( option = SignalOptions::xml_to_option(node) );
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
  node.content = xmldoc->content->first_node("value");
  BOOST_REQUIRE_NO_THROW( option = SignalOptions::xml_to_option(node) );
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
  node.content = xmldoc->content->first_node("value");
  BOOST_CHECK_THROW( SignalOptions::xml_to_option(node), ProtocolError );

  //
  // 4. option mode (basic/adv)
  //
  XmlDoc doc;
  Map map( doc.add_node( Protocol::Tags::node_map() ) );

  XmlNode theAnswer = map.set_value("theAnswer", class_name<int>(), "42");
  XmlNode pi = map.set_value("pi", class_name<Real>(), "3.14159" );
  XmlNode euler = map.set_value("euler", class_name<Real>(), "2.71" );

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
  boost::shared_ptr< XmlDoc > xmldoc(new XmlDoc());
  boost::shared_ptr<Option> option;

  Map map(xmldoc->add_node("map"));

  XmlNode optBool = map.set_value("optBool", class_name<bool>(), "true");
  XmlNode optInt = map.set_value("optInt", class_name<int>(), "-15468");
  XmlNode optUint = map.set_value("optUint", class_name<Uint>(), "17513214");
  XmlNode optReal = map.set_value("optReal", class_name<Real>(), "3.14159");
  XmlNode optString = map.set_value("optString", class_name<std::string>(), std::string("I am a string value"));
  XmlNode optURI = map.set_value("optURI", class_name<URI>(),  "cpath:/");

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
  BOOST_CHECK_EQUAL( option->value<URI>().string(), std::string("cpath:/") );

  // 7. unknown type
  BOOST_CHECK_THROW( SignalOptions::xml_to_option(wrongOpt), ValueNotFound );
}

//////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( xml_to_option_uri_schemes )
{
  boost::shared_ptr< XmlDoc > xmldoc(new XmlDoc());
  Map map(xmldoc->add_node("map"));
  const char * tag = Protocol::Tags::attr_uri_schemes();

  XmlNode opt1 = map.set_value("opt1", class_name<URI>(), URI().string());
  XmlNode opt2 = map.set_value("opt2", class_name<URI>(), URI().string());
  XmlNode opt3 = map.set_value("opt3", class_name<URI>(), URI().string());
  XmlNode opt4 = map.set_value("opt4", class_name<URI>(), URI().string());
  XmlNode opt5 = map.set_value("opt5", class_name<URI>(), URI().string());

  // opt1 has no scheme attribute defined
  opt2.set_attribute(tag, "");                  // no scheme defined
  opt3.set_attribute(tag, "http");              // one scheme
  opt4.set_attribute(tag, "cpath,https,file");  // several schemes
  opt5.set_attribute(tag, "cpath,scheme");      // a wrong scheme

  boost::shared_ptr< OptionURI > opt;
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
  boost::shared_ptr< XmlDoc > xmldoc(new XmlDoc());
  Map map(xmldoc->add_node("map"));
  boost::shared_ptr<Option> option;
  std::vector<int> vectInt = list_of<int>(344646)(544684)(446454)
                                                        (878764)(646316);

  XmlNode optInt = map.set_value("optInt", class_name<int>(), "13");
  XmlNode optIntRestrList = map.set_value("optIntRestrList", class_name<int>(), "-15468");

  Map(optIntRestrList).set_array(Protocol::Tags::key_restricted_values(), class_name<int>(), option_vector_to_str(vectInt, " ; "), " ; ");

  // test without restricted list
  BOOST_CHECK_NO_THROW(option = SignalOptions::xml_to_option( optInt ) );
  BOOST_CHECK( !option->has_restricted_list() );

  // test with restricted list
  BOOST_CHECK_NO_THROW(option = SignalOptions::xml_to_option( optIntRestrList ) );

  BOOST_CHECK( option->has_restricted_list() );

  std::vector<boost::any> & restr_list = option->restricted_list();

  BOOST_CHECK_EQUAL( restr_list.size(), size_t( vectInt.size() ) );
  BOOST_CHECK_EQUAL( boost::any_cast<int>(restr_list[0]), vectInt[0] );
  BOOST_CHECK_EQUAL( boost::any_cast<int>(restr_list[1]), vectInt[1] );
  BOOST_CHECK_EQUAL( boost::any_cast<int>(restr_list[2]), vectInt[2] );
  BOOST_CHECK_EQUAL( boost::any_cast<int>(restr_list[3]), vectInt[3] );
  BOOST_CHECK_EQUAL( boost::any_cast<int>(restr_list[4]), vectInt[4] );
}

//////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( set_options_from_vector )
{
  std::vector<std::string> args(12);
  std::vector<std::string> args_mod(2);
  SignalOptions options;

  // single values
  args[0] = "my_bool:bool=true";
  args[1] = "my_uint:unsigned=7489";
  args[2] = "my_int:integer=-4567";
  args[3] = "my_real:real=3.1415";
  args[4] = "my_string:string=Hello World!";
  args[5] = "my_uri:uri=cpath:/Tools";

  // array values
  args[6] = "my_bool_array:array[bool]=true,false,true, true";
  args[7] = "my_uint_array:array[unsigned]=7489, 12,65,0";
  args[8] = "my_int_array:array[integer]=-4567,42, 730";
  args[9] = "my_real_array:array[real]=3.1415,2.71,0.99,-12.35";
  args[10] = "my_string_array:array[string]=Hello World!, VKI,COOLFluiD";
  args[11] = "my_uri_array:array[uri]=cpath:/Tools, http://coolfluidsrv.vki.ac.be/cdash,file:/usr/bin/gcc";

  // options to modify
  args_mod[0] = "my_int:integer=42";
  args_mod[1] = "my_real_array:array[real]=1.56, 7894.012, 32.768";

  options.set( args );

  BOOST_CHECK_EQUAL ( options.store.size(), args.size() );

  ///////////
  // I. Check the single options
  ///////////

  // 1. my_bool
  BOOST_CHECK ( options.check("my_bool") );
  BOOST_CHECK_EQUAL ( std::string(options["my_bool"].type()), std::string("bool") );
  BOOST_CHECK ( options["my_bool"].value<bool>() );

  // 2. my_uint
  BOOST_CHECK ( options.check("my_uint") );
  BOOST_CHECK_EQUAL ( std::string(options["my_uint"].type()), std::string("unsigned") );
  BOOST_CHECK_EQUAL ( options["my_uint"].value<Uint>(), 7489 );

  // 3. my_int
  BOOST_CHECK ( options.check("my_int") );
  BOOST_CHECK_EQUAL ( std::string(options["my_int"].type()), std::string("integer") );
  BOOST_CHECK_EQUAL ( options["my_int"].value<int>(), -4567 );

  // 4. my_real
  BOOST_CHECK ( options.check("my_real") );
  BOOST_CHECK_EQUAL ( std::string(options["my_real"].type()), std::string("real") );
  BOOST_CHECK_EQUAL ( options["my_real"].value<Real>(), 3.1415 );

  // 5. my_string
  BOOST_CHECK ( options.check("my_string") );
  BOOST_CHECK_EQUAL ( std::string(options["my_string"].type()), std::string("string") );
  BOOST_CHECK_EQUAL ( options["my_string"].value<std::string>(), std::string("Hello World!") );

  // 6. my_uri
  BOOST_CHECK ( options.check("my_uri") );
  BOOST_CHECK_EQUAL ( std::string(options["my_uri"].type()), std::string("uri") );
  BOOST_CHECK_EQUAL ( options["my_uri"].value<URI>().path(), URI("cpath:/Tools").path() );


  ///////////
  // II. Check the array options
  ///////////

  // 1. my_bool_array
  BOOST_CHECK ( options.check("my_bool_array") );
  BOOST_CHECK_EQUAL ( std::string(options["my_bool_array"].type()), std::string("array[bool]") );
  std::vector<bool> bool_array = options["my_bool_array"].value< std::vector<bool> >();
  BOOST_CHECK_EQUAL ( bool_array.size(), 4 );
  BOOST_CHECK ( bool_array[0] );
  BOOST_CHECK ( !bool_array[1] );
  BOOST_CHECK ( bool_array[2] );
  BOOST_CHECK ( bool_array[3] );

  // 2. my_uint_array
  BOOST_CHECK ( options.check("my_uint_array") );
  BOOST_CHECK_EQUAL ( std::string(options["my_uint_array"].type()), std::string("array[unsigned]") );
  std::vector<Uint> uint_array = options["my_uint_array"].value< std::vector<Uint> >();
  BOOST_CHECK_EQUAL ( uint_array.size(), 4 );
  BOOST_CHECK_EQUAL ( uint_array[0], 7489 );
  BOOST_CHECK_EQUAL ( uint_array[1], 12 );
  BOOST_CHECK_EQUAL ( uint_array[2], 65 );
  BOOST_CHECK_EQUAL ( uint_array[3], 0 );

  // 3. my_int_array
  BOOST_CHECK ( options.check("my_int_array") );
  BOOST_CHECK_EQUAL ( std::string(options["my_int_array"].type()), std::string("array[integer]") );
  std::vector<int> int_array = options["my_int_array"].value< std::vector<int> >();
  BOOST_CHECK_EQUAL ( int_array.size(), 3 );
  BOOST_CHECK_EQUAL ( int_array[0], -4567 );
  BOOST_CHECK_EQUAL ( int_array[1], 42 );
  BOOST_CHECK_EQUAL ( int_array[2], 730 );

  // 4. my_real_array
  BOOST_CHECK ( options.check("my_real_array") );
  BOOST_CHECK_EQUAL ( std::string(options["my_real_array"].type()), std::string("array[real]") );
  std::vector<Real> real_array = options["my_real_array"].value< std::vector<Real> >();
  BOOST_CHECK_EQUAL ( real_array.size(), 4 );
  BOOST_CHECK_EQUAL ( real_array[0], 3.1415 );
  BOOST_CHECK_EQUAL ( real_array[1], 2.71 );
  BOOST_CHECK_EQUAL ( real_array[2], 0.99 );
  BOOST_CHECK_EQUAL ( real_array[3], -12.35 );

  // 5. my_string_array
  BOOST_CHECK ( options.check("my_string_array") );
  BOOST_CHECK_EQUAL ( std::string(options["my_string_array"].type()), std::string("array[string]") );
  std::vector<std::string> string_array = options["my_string_array"].value< std::vector<std::string> >();
  BOOST_CHECK_EQUAL ( string_array.size(), 3 );
  BOOST_CHECK_EQUAL ( string_array[0], std::string("Hello World!") );
  BOOST_CHECK_EQUAL ( string_array[1], std::string("VKI") );
  BOOST_CHECK_EQUAL ( string_array[2], std::string("COOLFluiD") );

  // 6. my_uri_array
  BOOST_CHECK ( options.check("my_uri_array") );
  BOOST_CHECK_EQUAL ( std::string(options["my_uri_array"].type()), std::string("array[uri]") );
  std::vector<URI> uri_array = options["my_uri_array"].value< std::vector<URI> >();
  BOOST_CHECK_EQUAL ( uri_array.size(), 3 );
  BOOST_CHECK_EQUAL ( uri_array[0].path(), URI("cpath:/Tools").path() );
  BOOST_CHECK_EQUAL ( uri_array[1].path(), URI("http://coolfluidsrv.vki.ac.be/cdash").path() );
  BOOST_CHECK_EQUAL ( uri_array[2].path(), URI("file:/usr/bin/gcc").path() );

  ////////////
  // III. Modify options
  ////////////

  options.set( args_mod );

  BOOST_CHECK_EQUAL ( options.store.size(), args.size() );

  // 1. my_int
  BOOST_CHECK_EQUAL ( options["my_int"].value<int>(), 42 );

  // 2. my_real_array
  real_array = options["my_real_array"].value< std::vector<Real> >();
  BOOST_CHECK_EQUAL ( real_array.size(), 3 );
  BOOST_CHECK_EQUAL ( real_array[0], 1.56 );
  BOOST_CHECK_EQUAL ( real_array[1], 7894.012 );
  BOOST_CHECK_EQUAL ( real_array[2], 32.768 );
}

//////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

/////////////////////////////////////////////////////////////////////////////
