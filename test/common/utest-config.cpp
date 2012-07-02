// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF log level filter"

#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/foreach.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "common/OptionArray.hpp"
#include "common/BasicExceptions.hpp"
#include "common/Group.hpp"
#include "common/Log.hpp"
#include "common/URI.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionURI.hpp"
#include "common/OptionT.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalFrame.hpp"
#include "common/XML/XmlDoc.hpp"
#include "common/XML/FileOperations.hpp"

#include "test/common/DummyComponents.hpp"

using namespace std;
using namespace boost;
using namespace boost::assign;

using namespace cf3;
using namespace cf3::common;

/////////////////////////////////////////////////////////////////////////////////////

class MyC : public Component {
public:
  static string type_name() { return "MyC"; }

  typedef boost::shared_ptr<MyC> Ptr;
  typedef boost::shared_ptr<MyC const> CosntPtr;

  private: // data

    std::string m_str;
    int m_i;
    Handle<CConcrete1> m_component;
    Handle<CConcrete1> m_component_lnk;
    Handle<CConcrete1> m_component_lnk2;

  public: // functions

  const Handle<CConcrete1>& comp() { return m_component_lnk; }

  MyC ( const std::string& name ) :  Component(name)
  {
    // POD's (plain old data)
    options().add ( "OptBool", false ).description("bool option");
    options().add ( "OptInt", -5 ).description("int option");
    options().add ( "OptUInt", 10u ).description("Uint option");
    options().add ( "OptReal", 0.0 ).description("real option");
    options().add ( "OptStr", std::string("LOLO") ).description("string option");
    options().add ( "OptURI", URI("cpath:/lolo") ).description( "URI option");

    // vector of POD's
    std::vector<int> def;
    def += 1,2,3,4,5,6,7,8,9; /* uses boost::assign */
    options().add( "VecInt", def ).description("vector ints option");

    // vector of POD's
    std::vector< std::string > defs;
    defs += "lolo","koko";     /* uses boost::assign */
    options().add( "VecStr", defs ).description("vector strs option");;

//    option("OptInt").set_value(10);

    options()["OptInt"].link_to( &m_i );

    options().link_to_parameter ( "OptStr", &m_str );

    options()["OptBool"].attach_trigger( boost::bind ( &MyC::config_bool,  this ) );
    options()["OptInt"].attach_trigger ( boost::bind ( &MyC::config_int,   this ) );
    options()["OptStr"].attach_trigger ( boost::bind ( &MyC::config_str,   this ) );
    options()["VecInt"].attach_trigger ( boost::bind ( &MyC::config_vecint,this ) );
    options()["OptURI"].attach_trigger ( boost::bind ( &MyC::config_uri,   this ) );

    std::vector<int> vi = options().value< std::vector<int> >("VecInt");
//    for (Uint i = 0; i < vi.size(); ++i)
//      CFinfo << "vi[" << i << "] : " << vi[i] << "\n" << CFendl;

    options().add( "OptC", Handle<CConcrete1>() )
        .description("component option");
    options().link_to_parameter ( "OptC", &m_component_lnk );
    boost::shared_ptr<Option> opt2 (new OptionComponent<CConcrete1>("OptC2", Handle<CConcrete1>()));
    opt2->description("component option");
    options().add(opt2).link_to( &m_component_lnk2 ).mark_basic();

    Option& opt3 = options().add("OptC3",m_component);
    opt3.description("component option");
    CFinfo << opt3.value_str() << CFendl;
  };


  void config_bool ()
  {
    BOOST_CHECK(options().value<bool>("OptBool") == boost::any_cast<bool>(options().option("OptBool").value()));
  }

  void config_int ()
  {
    BOOST_CHECK(options().value<int>("OptInt") == boost::any_cast<int>(options().option("OptInt").value()));
  }

  void config_str ()
  {
    BOOST_CHECK(options().value<std::string>("OptStr") == boost::any_cast<std::string>(options().option("OptStr").value()));
  }

  void config_vecint ()
  {
    BOOST_CHECK(options().value< std::vector<int> >("VecInt") == boost::any_cast< std::vector<int> >(options().option("VecInt").value()));
  }

  void config_uri ()
  {
    BOOST_CHECK(options().value<URI>("OptURI") == boost::any_cast<URI>(options().option("OptURI").value()));
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

  boost::shared_ptr<MyC> pm ( allocate_component<MyC>("LOLO") );

//  CFinfo << "ending\n" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( configure )
{
  using namespace rapidxml;

  boost::gregorian::date today = boost::gregorian::day_clock::local_day();
  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

    CFinfo << "starting [" << today << "] [" << now << "]" << CFendl;


  boost::shared_ptr<MyC> pm ( allocate_component<MyC>("LOLO") );

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

  BOOST_CHECK ( pm->options().value<bool>("OptBool") );
  BOOST_CHECK_EQUAL ( pm->options().option("OptBool").value_str() , "true" );

  BOOST_CHECK_EQUAL ( pm->options().value<int>("OptInt"),   -156  );
  BOOST_CHECK_EQUAL ( pm->options().option("OptInt").value_str() ,  "-156" );

  BOOST_CHECK_EQUAL ( pm->options().value<Uint>("OptUInt"), (Uint)  134  );
  BOOST_CHECK_EQUAL ( pm->options().option("OptUInt").value_str()  ,  "134" );

  BOOST_CHECK_EQUAL ( pm->options().value<Real>("OptReal"),   6.4564E+5  );
//  BOOST_CHECK_EQUAL ( pm->options().option("OptReal").value_str()  ,  "6.4564E+5" );

  BOOST_CHECK_EQUAL ( pm->options().value<std::string>("OptStr"), "lolo" );
  BOOST_CHECK_EQUAL ( pm->options().option("OptStr").value_str(),          "lolo" );

  std::vector<int> vecint(3);
  vecint[0]=2;
  vecint[1]=8;
  vecint[2]=9;
  BOOST_CHECK ( pm->options().value<std::vector<int> >("VecInt") ==  vecint);

  std::vector<std::string> vecstr(2);
  vecstr[0]="aabbcc";
  vecstr[1]="ddeeff";
  BOOST_CHECK ( pm->options().value<std::vector<std::string> >("VecStr") ==  vecstr);

  CFinfo << "ending" << CFendl;

}

////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE( configure_component_path )
{
  // Setup a little data-structure
  boost::shared_ptr<Group> root = allocate_component<Group>("root");
  Handle<CConcrete1> component1 = root->create_component<CConcrete1>("component1");
  Handle<CConcrete1> component2 = root->create_component<CConcrete1>("component2");

  // Configure component 1 without XML (It could also be done with xml)
  component1->options().set("MyRelativeFriend",URI("cpath:../component2"));
  component1->options().set("MyAbsoluteFriend",URI("cpath:/component2"));

  // Check if everything worked OK.
  URI absolute_friend_path = component1->options().value<URI>("MyAbsoluteFriend");
  Handle<CConcrete1> absolute_friend(component1->access_component(absolute_friend_path));
  BOOST_CHECK_EQUAL(absolute_friend->name(),"component2");

  URI relative_friend_path = component1->options().value<URI>("MyRelativeFriend");
  Handle<CConcrete1> relative_friend(component1->access_component(relative_friend_path));
  BOOST_CHECK_EQUAL(relative_friend->name(),"component2");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( optionComponent )
{
  // Setup a little data-structure
  Component& root = Core::instance().root();
  Handle<MyC> component1 = root.create_component<MyC>("component1");
  Handle<CConcrete1> component2 = root.create_component<CConcrete1>("component2");

  // Configure component 1 without XML (It could also be done with xml)
  component1->options().set("OptC",component2);

  BOOST_CHECK( component1->comp() == component2 );
  BOOST_CHECK(component1->options().value< Handle<CConcrete1> >("OptC") == component2);
  // // Check if everything worked OK.
  // URI absolute_friend_path = component1->option("MyAbsoluteFriend").value<URI>();
  // Handle<CConcrete1> absolute_friend = component1->access_component(absolute_friend_path)->as_ptr<CConcrete1>();
  // BOOST_CHECK_EQUAL(absolute_friend->name(),"component2");
  //
  // URI relative_friend_path = component1->option("MyRelativeFriend").value<URI>();
  // Handle<CConcrete1> relative_friend = component1->access_component(relative_friend_path)->as_ptr<CConcrete1>();
  // BOOST_CHECK_EQUAL(relative_friend->name(),"component2");
}

////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
