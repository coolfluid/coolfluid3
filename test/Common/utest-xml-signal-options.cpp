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

#include "Common/OptionArray.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalOptions.hpp"

//#include "Common/Log.hpp"
//#include "Common/URI.hpp"

//#include "Common/XML/SignalFrame.hpp"
//#include "Common/XML/Protocol.hpp"
//#include "Common/XML/XmlDoc.hpp"
//#include "Common/XML/FileOperations.hpp"

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

  SignalOptionList options( frame );

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

BOOST_AUTO_TEST_SUITE_END()

/////////////////////////////////////////////////////////////////////////////
