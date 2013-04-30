// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for XML maps manipulation"

#include "rapidxml/rapidxml.hpp"
#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/URI.hpp"

#include "common/XML/SignalFrame.hpp"
#include "common/XML/Protocol.hpp"
#include "common/XML/XmlDoc.hpp"
#include "common/XML/FileOperations.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( XmlSignalFrame_TestSuite )

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( contructors )
{
  XmlDoc first_doc;

  std::string str_second_map = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                               "<frame type=\"signal\">"
                               "  <map>"
                               "    <value key=\"map_A\">"
                               "      <map/>"
                               "    </value>"
                               "    <value key=\"not_a_map\">"
                               "      <int>0</int>"
                               "    </value>"
                               "    <value key=\"map_B\">"
                               "      <map/>"
                               "    </value>"
                               "  </map>"
                               "</frame>";

  boost::shared_ptr<XmlDoc> second_doc = XML::parse_string(str_second_map);

  // 1. the node does not contain any map
  SignalFrame first_frame ( first_doc );
  rapidxml::xml_node<>* map_node = first_doc.content->first_node( Protocol::Tags::node_map() );
  BOOST_CHECK ( cf3::is_not_null(map_node) );
  BOOST_CHECK_EQUAL ( map_node, first_frame.main_map.content.content );


  // 2. the node contains a non-empty map
  SignalFrame second_frame ( second_doc.get()->content->first_node( "frame" ) );

  // "map_A" and "map_B" should have been recognized has maps, but not "not_a_map"
  BOOST_CHECK ( second_frame.has_map("map_A") );
  BOOST_CHECK ( second_frame.has_map("map_B") );
  BOOST_CHECK ( !second_frame.has_map("not_a_map") );

  // 3. test the constructor that builds a new document
  URI sender("cpath:/sender");
  URI receiver("cpath:/receiver");
  SignalFrame third_frame ( "theTarget", sender, receiver);

  rapidxml::xml_node<>* frame_node = third_frame.main_map.content.content->parent();
  rapidxml::xml_attribute<>* tmp_attr;

  BOOST_CHECK ( frame_node != nullptr );
  BOOST_CHECK_EQUAL ( std::string(frame_node->name()), std::string( Protocol::Tags::node_frame() ));

  tmp_attr = frame_node->first_attribute( "target" );
  BOOST_CHECK ( tmp_attr != nullptr );
  BOOST_CHECK_EQUAL ( std::string(tmp_attr->value()), std::string("theTarget") );

  tmp_attr = frame_node->first_attribute( "sender" );
  BOOST_CHECK ( tmp_attr != nullptr );
  BOOST_CHECK_EQUAL ( URI(tmp_attr->value()).string(), sender.string() );

  tmp_attr = frame_node->first_attribute( "receiver" );
  BOOST_CHECK ( tmp_attr != nullptr );
  BOOST_CHECK_EQUAL ( URI(tmp_attr->value()).string(), receiver.string());
}

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( map )
{
  URI sender("cpath:/sender");
  URI receiver("cpath:/receiver");
  SignalFrame frame ( "theTarget", sender, receiver);

  BOOST_CHECK ( !frame.has_map("MyMap") );

  frame.map("MyMap"); // creates the map

  BOOST_CHECK ( frame.has_map("MyMap") );
}

/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

/////////////////////////////////////////////////////////////////////////////
