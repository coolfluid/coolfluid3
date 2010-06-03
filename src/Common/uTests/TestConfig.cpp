#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/foreach.hpp>

#include <boost/assign/std/vector.hpp>


#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "Common/ConfigObject.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/Log.hpp"
#include "Common/Component.hpp"

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

  static void defineConfigOptions ( OptionList& options )
  {

    // POD's (plain old data)
    options.add< OptionT<bool> >            ( "OptBool", "bool option"   , false  );
    options.add< OptionT<int> >             ( "OptInt",  "int option"    , -5     );
    options.add< OptionT<Uint> >            ( "OptUInt", "int option"    , 10     );
    options.add< OptionT<std::string> >     ( "OptStr",  "string option" , "LOLO" );

    // vector of POD's
    std::vector<int> def;
    def += 1,2,3,4,5,6,7,8,9; /* uses boost::assign */
    options.add< OptionArray< int >  >  ( "VecInt",  "vector ints option" , def );

    // vector of POD's
    std::vector< std::string > defs;
    defs += "lolo","koko";     /* uses boost::assign */
    options.add< OptionArray< std::string >  >   ( "VecStr",  "vector strs option" , defs );

    // option for componets
     options.add< OptionComponent >     ( "Comp",  "component option" , "CLink" );
  }

  MyC ()
  {
    addConfigOptionsTo<MyC>();

//    option("OptInt").set_value(10);

    option("OptInt")->link_to( &m_i );

    link_to_parameter ( "OptStr", &m_str );

    option("OptBool")->attach_processor( boost::bind ( &MyC::config_bool,  this ) );
    option("OptInt")->attach_processor ( boost::bind ( &MyC::config_int,   this ) );
    option("OptStr")->attach_processor ( boost::bind ( &MyC::config_str,   this ) );
    option("VecInt")->attach_processor ( boost::bind ( &MyC::config_vecint,this ) );

    std::vector<int> vi = option("VecInt")->value< std::vector<int> >();
//    for (Uint i = 0; i < vi.size(); ++i)
//      CFinfo << "vi[" << i << "] : " << vi[i] << "\n" << CFflush;

    option("Comp")->attach_processor ( boost::bind ( &MyC::config_comp,this ) );
  };

  void config_bool ()
  {
    boost::any value = option("OptBool")->value();
    bool b = boost::any_cast<bool>(value);
//    CFinfo << "config bool [" << Common::StringOps::to_str(b) << "]\n" << CFflush;
  }

  void config_int ()
  {
    Uint i = option("OptInt")->value<int>();
//    CFinfo << "config int [" <<  i << "]\n" << CFflush;
  }

  void config_str ()
  {
    std::string s; option("OptStr")->put_value(s);
//    CFinfo << "config str [" << s << "]\n" << CFflush;
  }

  void config_vecint ()
  {
    std::vector<int> vi; option("VecInt")->put_value(vi);
//    BOOST_FOREACH ( int i, vi )
//        CFinfo << "config vi [" << i << "]\n" << CFflush;
  }

  void config_comp ()
  {
    std::string n; option("Comp")->put_value(n);
//    CFinfo << "config COMPONENT [" << n << "]\n" << CFflush;
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

//  CFinfo << "starting [" << today << "] [" << now << "]\n" << CFflush;

  boost::shared_ptr<MyC> pm ( new MyC );

//  CFinfo << "ending\n" << CFflush;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( defineConfigOptions )
{
  boost::gregorian::date today = boost::gregorian::day_clock::local_day();
  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

//  CFinfo << "starting [" << today << "] [" << now << "]\n" << CFflush;

  OptionList ll;

  MyC::defineConfigOptions(ll);

//  CFinfo << "ending\n" << CFflush;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( configure )
{
  using namespace rapidxml;

  boost::gregorian::date today = boost::gregorian::day_clock::local_day();
  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

//  CFinfo << "starting [" << today << "] [" << now << "]\n" << CFflush;

  boost::shared_ptr<MyC> pm ( new MyC );

  std::string text = (
      "<MyC>"
      ""
      "<bool       key=\"OptBool\">    1 </bool>"
      "<integer    key=\"OptInt\" > -156 </integer>"
      "<integer    key=\"OptUint\" > 134 </integer>"
      "<real       key=\"OptReal\" > 6.4564E+5 </real>"
      "<string     key=\"OptStr\" > lolo </bool>"
      ""
      "<ignore>   popo  </ignore>"
      ""
      "<path      key=\"OptPath\"> /opt/path </path>"
      ""
      "<date      key=\"OptDate\"> 2010-05-24 </date>"
      ""
      "<data    key=\"OptData\"  > PEKBpYGlmYFCPA== </data>"
      ""
      "<Params key=\"OptList\" desc=\"sub set\" mode=\"advanced\" >"
      "     <integer  key=\"mi\">     2 </integer>"
      "     <real     key=\"mr\">     8 </real>"
      "     <bool     key=\"mb\">     1 </bool>"
      "</Params>"
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
      "<component key=\"OptComp\" >"
      "  <string key=\"name\"> MyNewton </string>"
      "  <string key=\"atype\"> CIterativeMethod </string>"
      "  <string key=\"ctype\"> Newton </string>"
      "</component>"
      ""
      "</MyC>"
   );

  xml_document<> doc;    // character type defaults to char
  char* ctext = doc.allocate_string(text.c_str());
  doc.parse< parse_no_data_nodes |
             parse_trim_whitespace |
             parse_normalize_whitespace >(ctext);

  pm->configure(*doc.first_node());

//  CFinfo << "ending\n" << CFflush;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
