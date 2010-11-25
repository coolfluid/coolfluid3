// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/foreach.hpp>
#include <boost/assign/std/vector.hpp>


#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "Common/ConfigObject.hpp"
#include "Common/OptionArray.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/Log.hpp"
#include "Common/XmlHelpers.hpp"
#include "Common/CRoot.hpp"
#include "Common/URI.hpp"

#include "Common/uTests/DummyComponents.hpp"

using namespace std;
using namespace boost;
using namespace boost::assign;

using namespace CF;
using namespace CF::Common;

/////////////////////////////////////////////////////////////////////////////////////

class MyC : public ConfigObject {

  private: // data

    std::string m_str;
    int m_i;

  public: // functions

  static void define_config_properties ( PropertyList& options )
  {

    // POD's (plain old data)
    options.add_option< OptionT<bool> >            ( "OptBool", "bool option"   , false  );
    options.add_option< OptionT<int> >             ( "OptInt",  "int option"    , -5     );
    options.add_option< OptionT<Uint> >            ( "OptUInt", "int option"    , 10     );
    options.add_option< OptionT<Real> >            ( "OptReal", "real option"   , 0.0   );
    options.add_option< OptionT<std::string> >     ( "OptStr",  "string option" , "LOLO" );
    options.add_option< OptionT<URI> >             ( "OptURI",  "URI option"    , "cpath://lolo" );

    // vector of POD's
    std::vector<int> def;
    def += 1,2,3,4,5,6,7,8,9; /* uses boost::assign */
    options.add_option< OptionArrayT< int >  >  ( "VecInt",  "vector ints option" , def );

    // vector of POD's
    std::vector< std::string > defs;
    defs += "lolo","koko";     /* uses boost::assign */
    options.add_option< OptionArrayT< std::string >  >   ( "VecStr",  "vector strs option" , defs );
  }

  MyC ()
  {
    add_options_to<MyC>();

//    option("OptInt").set_value(10);

    m_property_list["OptInt"].as_option().link_to( &m_i );

    link_to_parameter ( "OptStr", &m_str );

    m_property_list["OptBool"].as_option().attach_trigger( boost::bind ( &MyC::config_bool,  this ) );
    m_property_list["OptInt"].as_option().attach_trigger ( boost::bind ( &MyC::config_int,   this ) );
    m_property_list["OptStr"].as_option().attach_trigger ( boost::bind ( &MyC::config_str,   this ) );
    m_property_list["VecInt"].as_option().attach_trigger ( boost::bind ( &MyC::config_vecint,this ) );
    m_property_list["OptURI"].as_option().attach_trigger ( boost::bind ( &MyC::config_uri,   this ) );

    std::vector<int> vi = property("VecInt").value< std::vector<int> >();
//    for (Uint i = 0; i < vi.size(); ++i)
//      CFinfo << "vi[" << i << "] : " << vi[i] << "\n" << CFendl;

  };

  void config_bool ()
  {
    boost::any value = property("OptBool").value();
    // bool b = boost::any_cast<bool>(value);
    // CFinfo << "config bool [" << Common::String::to_str(b) << "]\n" << CFendl;
  }

  void config_int ()
  {
    // Uint i = option("OptInt")->value<int>();
    // CFinfo << "config int [" <<  i << "]\n" << CFendl;
  }

  void config_str ()
  {
    std::string s; property("OptStr").put_value(s);
//    CFinfo << "config str [" << s << "]\n" << CFendl;
  }

  void config_vecint ()
  {
    std::vector<int> vi; property("VecInt").put_value(vi);
//    BOOST_FOREACH ( int i, vi )
//        CFinfo << "config vi [" << i << "]\n" << CFendl;
  }

  void config_comp ()
  {
    CAbstract::Ptr abstract_component;
    property("OptComp").put_value(abstract_component);
  }

  void config_uri ()
  {
    URI uri; property("OptURI").put_value(uri);
    //    CFinfo << "config str [" << s << "]\n" << CFendl;
  }

};

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( Config_TestSuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( add_options_to )
{
  boost::gregorian::date today = boost::gregorian::day_clock::local_day();
  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

//  CFinfo << "starting [" << today << "] [" << now << "]\n" << CFendl;

  boost::shared_ptr<MyC> pm ( new MyC );

//  CFinfo << "ending\n" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( define_config_properties )
{
  boost::gregorian::date today = boost::gregorian::day_clock::local_day();
  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

//  CFinfo << "starting [" << today << "] [" << now << "]\n" << CFendl;

  PropertyList ll;

  MyC::define_config_properties(ll);

//  CFinfo << "ending\n" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( configure )
{
  using namespace rapidxml;

  boost::gregorian::date today = boost::gregorian::day_clock::local_day();
  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

    CFinfo << "starting [" << today << "] [" << now << "]" << CFendl;


  boost::shared_ptr<MyC> pm ( new MyC );

  std::string text = (
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
      "<cfxml version=\"1.0\">"
      "<signal>"
      " <map>"
      "  <value key=\"options\">"
      "   <map>"
      ""
      "<value  key=\"OptBool\"> <bool> 1 </bool> </value>"
      "<value  key=\"OptInt\" > <integer> -156 </integer> </value>"
      "<value  key=\"OptUInt\" > <unsigned> 134 </unsigned> </value>"
      "<value  key=\"OptReal\" > <real> 6.4564E+5 </real> </value>"
      "<value  key=\"OptStr\" > <string> lolo </string> </value>"
      ""
      "<ignore>   popo  </ignore>"
      ""
      "<value      key=\"OptPath\"> <path> /opt/path </path> </value>"
      ""
      "<value      key=\"OptDate\"> <date> 2010-05-24 </date> </value>"
      ""
      "<value   key=\"OptData\"  > <data> PEKBpYGlmYFCPA== </data> </value>"
      ""
      "<map key=\"OptList\" desc=\"sub set\" mode=\"advanced\" >"
      "     <value key=\"mi\"> <integer> 2 </integer> </value>"
      "     <value key=\"mr\"> <real> 8 </real> </value>"
      "     <value key=\"mb\"> <bool> 1 </bool> </value>"
      "</map>"
      ""
      "<array key=\"VecInt\" type=\"integer\" size=\"3\" >"
      "  <e> 2 </e>"
      "  <e> 8 </e>"
      "  <e> 9 </e>"
      "</array>"
      ""
      "<array key=\"VecStr\" type=\"string\" size=\"2\" >"
      "  <e> aabbcc </e>"
      "  <e> ddeeff </e>"
      "</array>"
      ""
//      "<map key=\"OptComp\" >"
//      "  <value key=\"name\"> <string> Abstract </string> </value>"
//      "  <value key=\"atype\"> <string> CAbstract </string> </value>"
//      "  <value key=\"ctype\"> <string> Concrete2 </string> </value>"

//      "</map>"
      ""
      "   </map>"
      "  </value>"
      " </map>"
      "</signal>"
      "</cfxml>"
   );

  boost::shared_ptr<XmlDoc> xml = XmlOps::parse(text);

  XmlNode& doc   = *XmlOps::goto_doc_node(*xml.get());
  XmlNode& frame = *XmlOps::first_frame_node( doc );

//  CFinfo << "FRAME [" << frame.name() << "]" << CFendl;


  // By default the OptComp is set to a Concrete1 specialization of CAbstract
  BOOST_CHECK_EQUAL ( pm->property("OptComp").value<CAbstract::Ptr>()->type(), "CConcrete1" );

  pm->configure( frame );

  BOOST_CHECK_EQUAL ( pm->property("OptBool").value<bool>(), true  );
  BOOST_CHECK_EQUAL ( pm->property("OptBool").value_str() , "true" );

  BOOST_CHECK_EQUAL ( pm->property("OptInt").value<int>(),   -156  );
  BOOST_CHECK_EQUAL ( pm->property("OptInt").value_str() ,  "-156" );

  BOOST_CHECK_EQUAL ( pm->property("OptUInt").value<Uint>(), (Uint)  134  );
  BOOST_CHECK_EQUAL ( pm->property("OptUInt").value_str()  ,  "134" );

  BOOST_CHECK_EQUAL ( pm->property("OptReal").value<Real>(),   6.4564E+5  );
//  BOOST_CHECK_EQUAL ( pm->property("OptReal").value_str()  ,  "6.4564E+5" );

  BOOST_CHECK_EQUAL ( pm->property("OptStr").value<std::string>(), "lolo" );
  BOOST_CHECK_EQUAL ( pm->property("OptStr").value_str(),          "lolo" );

  std::vector<int> vecint(3);
  vecint[0]=2;
  vecint[1]=8;
  vecint[2]=9;
  BOOST_CHECK ( pm->property("VecInt").value<std::vector<int> >() ==  vecint);

  std::vector<std::string> vecstr(2);
  vecstr[0]="aabbcc";
  vecstr[1]="ddeeff";
  BOOST_CHECK ( pm->property("VecStr").value<std::vector<std::string> >() ==  vecstr);

  // After configuring the OptComp is set to a Concrete2 specialization of CAbstract
//  BOOST_CHECK_EQUAL ( pm->option("OptComp")->value<CAbstract::Ptr>()->type(), "CConcrete2" );

  CFinfo << "ending" << CFendl;

}

////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE( configure_component_path )
{
  // Setup a little data-structure
  CRoot::Ptr root = CRoot::create("root");
  CConcrete1::Ptr component1 = root->create_component_type<CConcrete1>("component1");
  CConcrete1::Ptr component2 = root->create_component_type<CConcrete1>("component2");

  // Configure component 1 without XML (It could also be done with xml)
  component1->configure_property("MyRelativeFriend",URI("../component2"));
  component1->configure_property("MyAbsoluteFriend",URI("cpath://root/component2"));

  // Check if everything worked OK.
  CPath absolute_friend_path = component1->property("MyAbsoluteFriend").value<URI>();
  CConcrete1::Ptr absolute_friend = component1->look_component_type<CConcrete1>(absolute_friend_path);
  BOOST_CHECK_EQUAL(absolute_friend->name(),"component2");

  CPath relative_friend_path = component1->property("MyRelativeFriend").value<URI>();
  CConcrete1::Ptr relative_friend = component1->look_component_type<CConcrete1>(relative_friend_path);
  BOOST_CHECK_EQUAL(relative_friend->name(),"component2");
}

////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
