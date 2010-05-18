#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/foreach.hpp>

#include <boost/assign/std/vector.hpp>


#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "Common/ConfigObject.hpp"
#include "Common/OptionT.hpp"
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

class MyC : public ConfigObject
{
  public:

  static void defineConfigOptions ( OptionList& options )
  {

    // POD's (plain old data)
    options.add< OptionT<bool> >            ( "OptBool", "bool option"   , false  );
    options.add< OptionT<Uint> >            ( "OptInt",  "int option"    , 10     );
    options.add< OptionT<std::string> >     ( "OptStr",  "string option" , "LOLO" );

    // vector of POD's
    std::vector<int> def;
    def += 1,2,3,4,5,6,7,8,9; /* uses boost::assign */
    options.add< OptionT< std::vector<int> > >     ( "VecInt",  "vector ints option" , def );

    // option for componets
    options.add< OptionComponent >     ( "Comp",  "component option" , "CLink" );
  }

  MyC ()
  {
    addConfigOptionsTo(this);

//    option("OptInt").set_value(10);

//    option("OptInt").link_to ( &i ); // setParameter ( "OptInt", &i );

//    option("OptBool")->attach_processor( boost::bind ( &MyC::config_bool,  this ) );
//    option("OptInt")->attach_processor ( boost::bind ( &MyC::config_int,   this ) );
//    option("OptStr")->attach_processor ( boost::bind ( &MyC::config_str,   this ) );
//    option("VecInt")->attach_processor ( boost::bind ( &MyC::config_vecint,this ) );

    std::vector<int> vi = option("VecInt")->value< std::vector<int> >();
    for (Uint i = 0; i < vi.size(); ++i)
      CFinfo << "vi[" << i << "] : " << vi[i] << "\n" << CFendl;

    option("Comp")->attach_processor ( boost::bind ( &MyC::config_comp,this ) );
  };

  void config_bool ()
  {
    boost::any value = option("OptBool")->value();
    bool b = boost::any_cast<bool>(value);
    CFinfo << "config bool [" << Common::StringOps::to_str(b) << "]\n" << CFendl;
  }

  void config_int ()
  {
    CFinfo << "config int [" << option("OptInt")->value<Uint>() << "]\n" << CFendl;
  }

  void config_str ()
  {
    std::string s; option("OptStr")->put_value(s);
    CFinfo << "config str [" << s << "]\n" << CFendl;
  }

  void config_vecint ()
  {
    std::vector<int> vi; option("VecInt")->put_value(vi);
    BOOST_FOREACH ( int i, vi )
        CFinfo << "config vi [" << i << "]\n" << CFendl;
  }

  void config_comp ()
  {
    std::string n; option("Comp")->put_value(n);
    CFinfo << "config COMPONENT [" << n << "]\n" << CFendl;
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

  CFinfo << "starting [" << today << "] [" << now << "]\n" << CFendl;

  boost::shared_ptr<MyC> pm ( new MyC );

  CFinfo << "ending\n" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( defineConfigOptions )
{
  boost::gregorian::date today = boost::gregorian::day_clock::local_day();
  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

  CFinfo << "starting [" << today << "] [" << now << "]\n" << CFendl;

  OptionList ll;

  MyC::defineConfigOptions(ll);

  CFinfo << "ending\n" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( configure )
{
  using namespace rapidxml;

  boost::gregorian::date today = boost::gregorian::day_clock::local_day();
  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

  CFinfo << "starting [" << today << "] [" << now << "]\n" << CFendl;

  boost::shared_ptr<MyC> pm ( new MyC );

  std::string text = (
      "<MyC>"
      "<OptBool>     1  </OptBool>"
      "<OptInt>    134  </OptInt>"
      "<OptStr>   lolo  </OptStr>"
      "<Unused>   popo  </Unused>"
      "<VecInt>  2 8 9  </VecInt>"
      "<Comp>   CGroup  </Comp>"
      "</MyC>"
   );

  xml_document<> doc;    // character type defaults to char
  char* ctext = doc.allocate_string(text.c_str());
  doc.parse< parse_no_data_nodes >(ctext);

  pm->configure(doc.first_node());

  CFinfo << "ending\n" << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
