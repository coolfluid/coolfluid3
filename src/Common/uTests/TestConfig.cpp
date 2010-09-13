#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/foreach.hpp>

#include <boost/assign/std/vector.hpp>


#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "Common/CommonLib.hpp"
#include "Common/ConfigObject.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/Log.hpp"
#include "Common/Component.hpp"
#include "Common/XmlHelpers.hpp"
#include "Common/ObjectProvider.hpp"
#include "Common/CRoot.hpp"
#include "Common/URI.hpp"

using namespace std;
using namespace boost;
using namespace boost::assign;

using namespace CF;
using namespace CF::Common;

/////////////////////////////////////////////////////////////////////////////////////

class CAbstract : public Component
{

public: // typedefs

  /// provider
  typedef Common::ConcreteProvider < CAbstract,1 > PROVIDER;
  /// pointer to this type
  typedef boost::shared_ptr<CAbstract> Ptr;
  typedef boost::shared_ptr<CAbstract const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CAbstract ( const CName& name ) : Component(name)
  {
    BUILD_COMPONENT;
  }

  /// Virtual destructor
  virtual ~CAbstract() {}

  /// Get the class name
  static std::string type_name () { return "CAbstract"; }

  // --------- Configuration ---------

  static void defineConfigOptions ( Common::OptionList& options ) {}

  // --------- Specific functions to this component ---------

  virtual std::string type() { return type_name(); }

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( CAbstract* self ) {}

};

class CConcrete1 : public CAbstract
{

public: // typedefs

  /// pointer to this type
  typedef boost::shared_ptr<CConcrete1> Ptr;
  typedef boost::shared_ptr<CConcrete1 const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CConcrete1 ( const CName& name ) : CAbstract(name)
  {
    BUILD_COMPONENT;
  }

  /// Virtual destructor
  virtual ~CConcrete1() {}

  /// Get the class name
  static std::string type_name () { return "CConcrete1"; }

  // --------- Configuration ---------

  static void defineConfigOptions ( Common::OptionList& options )
  {
    URI def_path("cpath://");
    options.add< OptionT<URI> > ( "MyRelativeFriend", "a path to another component"   , def_path  );
    options.add< OptionT<URI> > ( "MyAbsoluteFriend", "a path to another component"   , def_path  );
  }

  // --------- Specific functions to this component ---------

  virtual std::string type() { return type_name(); }

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( CConcrete1* self ) {}

};

class CConcrete2 : public CAbstract
{

public: // typedefs

  /// pointer to this type
  typedef boost::shared_ptr<CConcrete2> Ptr;
  typedef boost::shared_ptr<CConcrete2 const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CConcrete2 ( const CName& name ) : CAbstract(name)
  {
    BUILD_COMPONENT;
  }

  /// Virtual destructor
  virtual ~CConcrete2() {}

  /// Get the class name
  static std::string type_name () { return "CConcrete2"; }

  // --------- Configuration ---------

  static void defineConfigOptions ( Common::OptionList& options ) {}

  // --------- Specific functions to this component ---------

  virtual std::string type() { return type_name(); }

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( CConcrete2* self ) {}

};


CF::Common::ObjectProvider < CConcrete1, CAbstract, CommonLib, NB_ARGS_1 >
aConcrete1ComponentProvider ( "Concrete1" );

CF::Common::ObjectProvider < CConcrete2, CAbstract, CommonLib, NB_ARGS_1 >
aConcrete2ComponentProvider ( "Concrete2" );

/////////////////////////////////////////////////////////////////////////////////////

class MyC : public ConfigObject {

  private: // data

    std::string m_str;
    int m_i;

  public: // functions

  static void defineConfigOptions ( OptionList& options )
  {

    // POD's (plain old data)
    options.add< OptionT<bool> >            ( "OptBool", "bool option"   , false  );
    options.add< OptionT<int> >             ( "OptInt",  "int option"    , -5     );
    options.add< OptionT<Uint> >            ( "OptUInt", "int option"    , 10     );
    options.add< OptionT<Real> >            ( "OptReal", "real option"   , 0.0   );
    options.add< OptionT<std::string> >     ( "OptStr",  "string option" , "LOLO" );
    options.add< OptionT<URI> >             ( "OptURI",  "URI option"    , "cpath://lolo" );

    // vector of POD's
    std::vector<int> def;
    def += 1,2,3,4,5,6,7,8,9; /* uses boost::assign */
    options.add< OptionArrayT< int >  >  ( "VecInt",  "vector ints option" , def );

    // vector of POD's
    std::vector< std::string > defs;
    defs += "lolo","koko";     /* uses boost::assign */
    options.add< OptionArrayT< std::string >  >   ( "VecStr",  "vector strs option" , defs );

    // option for componets
    options.add< OptionComponent<CAbstract > > ( "OptComp",  "abstract specialization" , "Concrete1" );
  }

  MyC ()
  {
    addConfigOptionsTo<MyC>();

//    option("OptInt").set_value(10);

    option("OptInt")->link_to( &m_i );

    link_to_parameter ( "OptStr", &m_str );

    option("OptBool")->attach_trigger( boost::bind ( &MyC::config_bool,  this ) );
    option("OptInt")->attach_trigger ( boost::bind ( &MyC::config_int,   this ) );
    option("OptStr")->attach_trigger ( boost::bind ( &MyC::config_str,   this ) );
    option("VecInt")->attach_trigger ( boost::bind ( &MyC::config_vecint,this ) );
    option("OptURI")->attach_trigger ( boost::bind ( &MyC::config_uri,   this ) );

    std::vector<int> vi = option("VecInt")->value< std::vector<int> >();
//    for (Uint i = 0; i < vi.size(); ++i)
//      CFinfo << "vi[" << i << "] : " << vi[i] << "\n" << CFendl;

    option("OptComp")->attach_trigger ( boost::bind ( &MyC::config_comp,this ) );
  };

  void config_bool ()
  {
    boost::any value = option("OptBool")->value();
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
    std::string s; option("OptStr")->put_value(s);
//    CFinfo << "config str [" << s << "]\n" << CFendl;
  }

  void config_vecint ()
  {
    std::vector<int> vi; option("VecInt")->put_value(vi);
//    BOOST_FOREACH ( int i, vi )
//        CFinfo << "config vi [" << i << "]\n" << CFendl;
  }

  void config_comp ()
  {
    CAbstract::Ptr abstract_component;
    option("OptComp")->put_value(abstract_component);
  }

  void config_uri ()
  {
    URI uri; option("OptURI")->put_value(uri);
    //    CFinfo << "config str [" << s << "]\n" << CFendl;
  }

};

////////////////////////////////////////////////////////////////////////////////

struct Config_Fixture
{
  /// common setup for each test case
  Config_Fixture()
  {
    // int*    argc = &boost::unit_test::framework::master_test_suite().argc;
    // char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~Config_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Config_TestSuite, Config_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( addConfigOptionsTo )
{
  boost::gregorian::date today = boost::gregorian::day_clock::local_day();
  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

//  CFinfo << "starting [" << today << "] [" << now << "]\n" << CFendl;

  boost::shared_ptr<MyC> pm ( new MyC );

//  CFinfo << "ending\n" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( defineConfigOptions )
{
  boost::gregorian::date today = boost::gregorian::day_clock::local_day();
  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

//  CFinfo << "starting [" << today << "] [" << now << "]\n" << CFendl;

  OptionList ll;

  MyC::defineConfigOptions(ll);

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
      " <valuemap>"
      "  <value key=\"options\">"
      "   <valuemap>"
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
      "<valuemap key=\"OptList\" desc=\"sub set\" mode=\"advanced\" >"
      "     <value key=\"mi\"> <integer> 2 </integer> </value>"
      "     <value key=\"mr\"> <real> 8 </real> </value>"
      "     <value key=\"mb\"> <bool> 1 </bool> </value>"
      "</valuemap>"
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
//      "<valuemap key=\"OptComp\" >"
//      "  <value key=\"name\"> <string> Abstract </string> </value>"
//      "  <value key=\"atype\"> <string> CAbstract </string> </value>"
//      "  <value key=\"ctype\"> <string> Concrete2 </string> </value>"

//      "</valuemap>"
      ""
      "   </valuemap>"
      "  </value>"
      " </valuemap>"
      "</signal>"
      "</cfxml>"
   );

  boost::shared_ptr<XmlDoc> xml = XmlOps::parse(text);

  XmlNode& doc   = *XmlOps::goto_doc_node(*xml.get());
  XmlNode& frame = *XmlOps::first_frame_node( doc );

  CFinfo << "FRAME [" << frame.name() << "]" << CFendl;


  // By default the OptComp is set to a Concrete1 specialization of CAbstract
  BOOST_CHECK_EQUAL ( pm->option("OptComp")->value<CAbstract::Ptr>()->type(), "CConcrete1" );

  pm->configure( frame );

  BOOST_CHECK_EQUAL ( pm->option("OptBool")->value<bool>(), true  );
  BOOST_CHECK_EQUAL ( pm->option("OptBool")->value_str() , "true" );

  BOOST_CHECK_EQUAL ( pm->option("OptInt")->value<int>(),   -156  );
  BOOST_CHECK_EQUAL ( pm->option("OptInt")->value_str() ,  "-156" );

  BOOST_CHECK_EQUAL ( pm->option("OptUInt")->value<Uint>(), (Uint)  134  );
  BOOST_CHECK_EQUAL ( pm->option("OptUInt")->value_str()  ,  "134" );

  BOOST_CHECK_EQUAL ( pm->option("OptReal")->value<Real>(),   6.4564E+5  );
//  BOOST_CHECK_EQUAL ( pm->option("OptReal")->value_str()  ,  "6.4564E+5" );

  BOOST_CHECK_EQUAL ( pm->option("OptStr")->value<std::string>(), "lolo" );
  BOOST_CHECK_EQUAL ( pm->option("OptStr")->value_str(),          "lolo" );

  std::vector<int> vecint(3);
  vecint[0]=2;
  vecint[1]=8;
  vecint[2]=9;
  BOOST_CHECK ( pm->option("VecInt")->value<std::vector<int> >() ==  vecint);

  std::vector<std::string> vecstr(2);
  vecstr[0]="aabbcc";
  vecstr[1]="ddeeff";
  BOOST_CHECK ( pm->option("VecStr")->value<std::vector<std::string> >() ==  vecstr);

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
  component1->configure_option("MyRelativeFriend",URI("../component2"));
  component1->configure_option("MyAbsoluteFriend",URI("cpath://root/component2"));

  // Check if everything worked OK.
  CPath absolute_friend_path = component1->option("MyAbsoluteFriend")->value<URI>();
  CConcrete1::Ptr absolute_friend = component1->look_component_type<CConcrete1>(absolute_friend_path);
  BOOST_CHECK_EQUAL(absolute_friend->name(),"component2");

  CPath relative_friend_path = component1->option("MyRelativeFriend")->value<URI>();
  CConcrete1::Ptr relative_friend = component1->look_component_type<CConcrete1>(relative_friend_path);
  BOOST_CHECK_EQUAL(relative_friend->name(),"component2");
}

////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
