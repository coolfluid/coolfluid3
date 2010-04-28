#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/property_map/property_map.hpp>
#include <boost/property_map/dynamic_property_map.hpp>


#include "Common/CF.hpp"
#include "Common/Log.hpp"

using namespace std;
using namespace boost;

using namespace CF;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

//class OptionValidation
//{
//public:
//};

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
  m_name(name),
  m_type(type),
  m_description(desc)
  {}

  std::string name() const { return m_name; }
  std::string type() const { return m_name; }
  std::string description() const { return m_description; }

protected:

  std::string m_name;
  std::string m_type;
  std::string m_description;

//  std::vector<OptionValidation> m_vals;
};

//------------------------------------------------------------------------------

template < typename TYPE >
    class OptionT : public Option
{
public:

//  typedef boost::function< void ( CLASS_TYPE*, TYPE ) > FUNC_TYPE;
//  typedef boost::shared_ptr< OptionT<TYPE> > Ptr;

  OptionT ( const std::string& name, const std::string& desc, TYPE def ) :
      Option(name,DEMANGLED_TYPEID(TYPE), desc),
//      m_processors(),
      m_default(def)
  {
    CFinfo
//        <<  "class [" << DEMANGLED_TYPEID(CLASS_TYPE) << "]"
        << " creating option ["
        << m_name << "] of type ["
        << m_type << "] w default ["
        << m_default << "] desc ["
        << m_description << "]\n" << CFendl;
  }

//  void attach_processor ( FUNC_TYPE proc ) { m_processors.push_back(proc); };

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

  //  Option::Ptr getOption() { m_options.get; }

private:

  OptionList m_option_list;

};

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

//------------------------------------------------------------------------------

class MyC : public ConfigObject
{
  public:

  MyC ()
  {
    addConfigOptionsTo(this);



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
//        ->attach_validation( is_positive() );
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
  CFinfo << "starting config\n" << CFendl;

  boost::shared_ptr<MyC> pm ( new MyC );

  CFinfo << "ending config\n" << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( defineConfigOptions )
{
  CFinfo << "starting config\n" << CFendl;

  OptionList ll;

  MyC::defineConfigOptions(ll);

  CFinfo << "ending config\n" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////


