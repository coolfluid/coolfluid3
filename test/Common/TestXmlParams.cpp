// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/XmlHelpers.hpp"
#include "Common/BasicExceptions.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

/////////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( XmlParams_TestSuite )

/////////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructors )
{
  std::string text = (
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
      "<cfxml version=\"1.0\">"
      "<signal>"
      " <map>"
      "  <value key=\"options\">"
      "   <map>"
      "    <value  key=\"OptBool\"> <bool> true </bool> </value>"
      "    <value  key=\"OptInt\" > <integer> -156 </integer> </value>"
      "    <value  key=\"OptUint\" > <unsigned> 134 </unsigned> </value>"
      "    <value  key=\"OptReal\" > <real> 6.4564E+5 </real> </value>"
      "    <value  key=\"OptStr\" > <string> lolo </string> </value>"
      "   </map>"
      "  </value>"
      " </map>"
      "</signal>"
      "</cfxml>"
   );

  boost::shared_ptr<XmlDoc> xmldoc = XmlOps::parse ( text );

  XmlNode* docnode = XmlOps::goto_doc_node(*xmldoc.get());
  BOOST_CHECK_NO_THROW( XmlParams params ( *docnode->first_node() ) );

}

/////////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( goto_doc_node )
{
  std::string text = (
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
   );

  boost::shared_ptr<XmlDoc> xmldoc = XmlOps::parse ( text );
  BOOST_CHECK_THROW(  XmlOps::goto_doc_node(*xmldoc.get()) , Common::XmlError );
}

/////////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( throw_get_option )
{
  std::string text = (
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
      "<cfxml version=\"1.0\">"
      "</cfxml>"
   );

  boost::shared_ptr<XmlDoc> xmldoc = XmlOps::parse ( text );
  XmlNode& nodedoc = *XmlOps::goto_doc_node(*xmldoc.get());
  XmlParams params ( nodedoc );

  BOOST_CHECK_THROW( params.get_option<int>("noint")  , Common::XmlError );
}

/////////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( get_option )
{
  std::string text = (
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
      "<cfxml version=\"1.0\">"
      "<signal>"
      " <map>"
      "  <value key=\"options\">"
      "   <map>"
      "    <value  key=\"OptBool\"> <bool> true </bool> </value>"
      "    <value  key=\"OptInt\" > <integer> -156 </integer> </value>"
      "    <value  key=\"OptUint\" > <unsigned> 134 </unsigned> </value>"
      "    <value  key=\"OptReal\" > <real> 6.4564E+5 </real> </value>"
      "    <value  key=\"OptStr\" > <string> lolo </string> </value>"
      "   </map>"
      "  </value>"
      " </map>"
      "</signal>"
      "</cfxml>"
   );

  boost::shared_ptr<XmlDoc> xmldoc = XmlOps::parse ( text );

  XmlNode& nodedoc = *XmlOps::goto_doc_node(*xmldoc.get());

  XmlParams params ( *nodedoc.first_node() );

//  XmlOps::print_xml_node(*doc.get());

  BOOST_REQUIRE_EQUAL ( params.get_option<bool>("OptBool") , true );

  BOOST_REQUIRE_EQUAL ( params.get_option<int>("OptInt"), -156 );

  BOOST_REQUIRE_EQUAL ( params.get_option<std::string>("OptStr") , "lolo" );

//  std::vector<Uint> v;
//  v += 2,8,9;
//  std::vector<Uint> cv = params.get_option<Uint>("VecInt");
//  BOOST_CHECK_EQUAL_COLLECTIONS( v.begin(), v.end(), cv.begin(), cv.end() );

  /// @todo how to access the nexted params?
}

/////////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

/////////////////////////////////////////////////////////////////////////////////////
