#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <boost/function.hpp>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "Common/CF.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/Log.hpp"

using namespace std;
using namespace boost;

using namespace CF;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

/// Definition of the ConfigKey type
typedef std::string ConfigKey;
/// Definition of the ConfigValue type
typedef std::string ConfigValue;
/// Definition of the map std::string to std::string type.
typedef std::map<ConfigKey,ConfigValue> ConfigMap;

class Option
{
public:

  typedef boost::shared_ptr<Option> Ptr;

  Option ( const std::string& name,
           const std::string& type,
           const std::string& desc ) :
  m_value(CFNULL),
  m_name(name),
  m_type(type),
  m_description(desc)
  {}

  std::string name() const { return m_name; }
  std::string type() const { return m_name; }
  std::string description() const { return m_description; }

  void link_to_value ( void *const value ) { m_value = value; }
  void attach_processor ( boost::function< void()> proc ) { m_processors.push_back(proc); }

  virtual void set_value (const std::string& value) = 0;

protected:

  void * m_value;

  std::string m_name;
  std::string m_type;
  std::string m_description;

  //  std::vector<OptionValidation> m_vals;

  std::vector< boost::function< void()> > m_processors;
};

//------------------------------------------------------------------------------

template < typename TYPE >
class OptionT : public Option
{
public:

  OptionT ( const std::string& name, const std::string& desc, TYPE def ) :
      Option(name,DEMANGLED_TYPEID(TYPE), desc),
      m_default(def)
  {
    CFinfo
        << " creating option ["
        << m_name << "] of type ["
        << m_type << "] w default ["
        << m_default << "] desc ["
        << m_description << "]\n" << CFendl;
  }

  virtual void set_value (const std::string& value_str )
  {
    TYPE * value = static_cast<TYPE*>(m_value);
    *value = Common::StringOps::from_str < TYPE > ( value_str );
  }

private:

  TYPE m_default;

};

//------------------------------------------------------------------------------

class OptionList
{
private:

  typedef std::map < std::string , Option::Ptr > OptionStorage_t;

public:

  template < typename TYPE >
  Option::Ptr addConfigOption(const std::string& name, const std::string& description, TYPE def )
  {
    cf_assert_desc ( "Class has already option with same name", m_options.find(name) == m_options.end() );
    Option::Ptr opt ( new OptionT<TYPE>(name, description, def ) );
    m_options.insert( std::make_pair(name, opt ) );
    return opt;
  }

  Option::Ptr getOption( const std::string& optname)
  {
    OptionStorage_t::iterator itr = m_options.find(optname);
    if ( itr != m_options.end() )
      return itr->second;
    else
      throw ValueNotFound(FromHere(), "Option with name [" + optname + "] not found" );
  }

public:

  /// storage of options
  OptionStorage_t m_options;
  
};

//------------------------------------------------------------------------------

class ConfigObject
{
public:

  /// Sets the config options by calling the defineConfigOptions
  /// This will add nested names to the options as opposed to addOptionsTo
  /// @param prt should be passed with the this pointer to help identify callee the CLASS type
  template <typename CLASS>
  void addConfigOptionsTo(const CLASS* ptr)
  {
    CLASS::defineConfigOptions(m_option_list);
  }

protected:

  Option::Ptr getOption( const std::string& optname )
  {
    return m_option_list.getOption(optname);
  }

private:

  OptionList m_option_list;

};


//------------------------------------------------------------------------------

class MyC : public ConfigObject
{
  public:

  MyC ()
  {
    addConfigOptionsTo(this);

    getOption("OptBool")->link_to_value(&b);
    getOption("OptInt")->link_to_value(&i);
    getOption("OptStr")->link_to_value(&s);

    getOption("OptBool")->attach_processor( boost::bind ( &MyC::config_bool, this ) );
    getOption("OptInt")->attach_processor ( boost::bind ( &MyC::config_int,  this ) );
    getOption("OptStr")->attach_processor ( boost::bind ( &MyC::config_str,  this ) );

  };

  void config_bool ()
  {
    CFinfo << "config bool\n" << CFendl;
  }

  void config_int ()
  {
    CFinfo << "config int" << CFendl;
  }

  void config_str ()
  {
    CFinfo << "config str\n" << CFendl;
  }


  static void defineConfigOptions ( OptionList& options )
  {
    options.addConfigOption< bool >         ( "OptBool", "bool option"   , false  );
    options.addConfigOption< Uint >         ( "OptInt",  "int option"    , 10     );
    options.addConfigOption< std::string >  ( "OptStr",  "string option" , "LOLO" );
  }

private:

  bool b;
  Uint i;
  std::string s;

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

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////


    //------------------------------------------------------------------------------

    //struct is_positive : public OptionValidation
    //{
    //  is_positive() {}
    //
    //  std::string desc () { return "Value must be positive"; }
    //
    //  template < typename TYPE >
    //      bool operator () ( TYPE v ) { return v > 0; }
    //};
