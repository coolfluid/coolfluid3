// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF log level filter"

#include <boost/test/unit_test.hpp>

#include <boost/foreach.hpp>
#include <boost/assign/std/vector.hpp>


#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "Common/OptionArray.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/URI.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionT.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/XmlDoc.hpp"
#include "Common/XML/FileOperations.hpp"

#include "test/Common/DummyComponents.hpp"

using namespace std;
using namespace boost;
using namespace boost::assign;

using namespace CF;
using namespace CF::Common;

/////////////////////////////////////////////////////////////////////////////////////

class MyC : public Component {
public:
  static string type_name() { return "MyC"; }

  typedef boost::shared_ptr<MyC> Ptr;
  typedef boost::shared_ptr<MyC const> CosntPtr;

  private: // data

    std::string m_str;
    int m_i;
    boost::weak_ptr<CConcrete1> m_component;

  public: // functions

  CConcrete1::Ptr comp() { return m_component.lock(); }

  MyC ( const std::string& name ) :  Component(name)
  {
    // POD's (plain old data)
    m_properties.add_option< OptionT<bool> >            ( "OptBool", "bool option"   , false  );
    m_properties.add_option< OptionT<int> >             ( "OptInt",  "int option"    , -5     );
    m_properties.add_option< OptionT<Uint> >            ( "OptUInt", "int option"    , 10     );
    m_properties.add_option< OptionT<Real> >            ( "OptReal", "real option"   , 0.0   );
    m_properties.add_option< OptionT<std::string> >     ( "OptStr",  "string option" , "LOLO" );
    m_properties.add_option< OptionURI >             ( "OptURI",  "URI option"    , URI("cpath://lolo") );

    // vector of POD's
    std::vector<int> def;
    def += 1,2,3,4,5,6,7,8,9; /* uses boost::assign */
    m_properties.add_option< OptionArrayT< int >  >  ( "VecInt",  "vector ints option" , def );

    // vector of POD's
    std::vector< std::string > defs;
    defs += "lolo","koko";     /* uses boost::assign */
    m_properties.add_option< OptionArrayT< std::string >  >   ( "VecStr",  "vector strs option" , defs );

//    option("OptInt").set_value(10);

    m_properties["OptInt"].as_option().link_to( &m_i );

    m_properties.link_to_parameter ( "OptStr", &m_str );

    m_properties["OptBool"].as_option().attach_trigger( boost::bind ( &MyC::config_bool,  this ) );
    m_properties["OptInt"].as_option().attach_trigger ( boost::bind ( &MyC::config_int,   this ) );
    m_properties["OptStr"].as_option().attach_trigger ( boost::bind ( &MyC::config_str,   this ) );
    m_properties["VecInt"].as_option().attach_trigger ( boost::bind ( &MyC::config_vecint,this ) );
    m_properties["OptURI"].as_option().attach_trigger ( boost::bind ( &MyC::config_uri,   this ) );

    std::vector<int> vi = property("VecInt").value< std::vector<int> >();
//    for (Uint i = 0; i < vi.size(); ++i)
//      CFinfo << "vi[" << i << "] : " << vi[i] << "\n" << CFendl;

    m_properties.add_option< OptionComponent<CConcrete1> >( "OptC", "component option", Core::instance().root()->full_path());
    m_properties.link_to_parameter ( "OptC", &m_component );
    Option::Ptr opt2 (new OptionComponent<CConcrete1>("OptC2","component option",Core::instance().root()->full_path()));
    m_properties.add_option(opt2)->link_to( &m_component )->mark_basic();
     Option::Ptr opt3 = m_properties.add_option
       (OptionComponent<CConcrete1>::create("OptC3","component option",&m_component));

     CFinfo << opt3->value_str() << CFendl;
     CFinfo << opt3->def_str() << CFendl;
    //   ->mark_basic()
    //   ->as_ptr<OptionComponent<CConcrete1> >();
    //
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

  boost::shared_ptr<MyC> pm ( new MyC("LOLO") );

//  CFinfo << "ending\n" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( configure )
{
  using namespace rapidxml;

  boost::gregorian::date today = boost::gregorian::day_clock::local_day();
  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

    CFinfo << "starting [" << today << "] [" << now << "]" << CFendl;


  boost::shared_ptr<MyC> pm ( new MyC("LOLO") );

  std::string text = (
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
      "<cfxml version=\"1.0\">"
      "<frame>"
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
      "<array key=\"VecInt\" type=\"integer\" size=\"3\" delimiter=\" ; \" > 2 ; 8 ; 9</array>"
      ""
      "<array key=\"VecStr\" type=\"string\" size=\"2\" delimiter=\" ; \">aabbcc ; ddeeff</array>"
      ""
      "   </map>"
      "  </value>"
      " </map>"
      "</frame>"
      "</cfxml>"
   );

  boost::shared_ptr<XmlDoc> xml = XML::parse_string(text);

  XmlNode doc = Protocol::goto_doc_node(*xml.get());
  SignalFrame frame( Protocol::first_frame_node( doc ) );

//  CFinfo << "FRAME [" << frame.name() << "]" << CFendl;

  pm->signal_configure( frame );

  BOOST_CHECK ( pm->property("OptBool").value<bool>() );
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

  CFinfo << "ending" << CFendl;

}

////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE( configure_component_path )
{
  // Setup a little data-structure
  CRoot::Ptr root = CRoot::create("root");
  CConcrete1::Ptr component1 = root->create_component<CConcrete1>("component1");
  CConcrete1::Ptr component2 = root->create_component<CConcrete1>("component2");

  // Configure component 1 without XML (It could also be done with xml)
  component1->configure_property("MyRelativeFriend",URI("cpath:../component2"));
  component1->configure_property("MyAbsoluteFriend",URI("cpath://root/component2"));

  // Check if everything worked OK.
  URI absolute_friend_path = component1->property("MyAbsoluteFriend").value<URI>();
  CConcrete1::Ptr absolute_friend = component1->access_component_ptr(absolute_friend_path)->as_ptr<CConcrete1>();
  BOOST_CHECK_EQUAL(absolute_friend->name(),"component2");

  URI relative_friend_path = component1->property("MyRelativeFriend").value<URI>();
  CConcrete1::Ptr relative_friend = component1->access_component_ptr(relative_friend_path)->as_ptr<CConcrete1>();
  BOOST_CHECK_EQUAL(relative_friend->name(),"component2");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( optionComponent )
{
  // Setup a little data-structure
  CRoot::Ptr root = Core::instance().root();
  MyC::Ptr component1 = root->create_component<MyC>("component1");
  CConcrete1::Ptr component2 = root->create_component<CConcrete1>("component2");

  // Configure component 1 without XML (It could also be done with xml)
  component1->configure_property("OptC",URI("cpath://Root/component2"));

  BOOST_CHECK( component1->comp() == component2 );
  // // Check if everything worked OK.
  // URI absolute_friend_path = component1->property("MyAbsoluteFriend").value<URI>();
  // CConcrete1::Ptr absolute_friend = component1->access_component_ptr(absolute_friend_path)->as_ptr<CConcrete1>();
  // BOOST_CHECK_EQUAL(absolute_friend->name(),"component2");
  //
  // URI relative_friend_path = component1->property("MyRelativeFriend").value<URI>();
  // CConcrete1::Ptr relative_friend = component1->access_component_ptr(relative_friend_path)->as_ptr<CConcrete1>();
  // BOOST_CHECK_EQUAL(relative_friend->name(),"component2");
}

////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
