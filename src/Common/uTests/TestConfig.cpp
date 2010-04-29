#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <boost/property_tree/detail/rapidxml.hpp>
#include <boost/any.hpp>

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
           const std::string& desc,
           boost::any def) :
  m_value(def),
  m_default(def),
  m_name(name),
  m_type(type),
  m_description(desc)
  {}

  std::string name() const { return m_name; }
  std::string type() const { return m_name; }
  std::string description() const { return m_description; }

  boost::any value() const { return m_value; }
  boost::any def() const { return m_default; }

  void attach_processor ( boost::function< void()> proc ) { m_processors.push_back(proc); }

protected:

  boost::any m_value;
  const boost::any m_default;

  std::string m_name;
  std::string m_type;
  std::string m_description;

  std::vector< boost::function< void()> > m_processors;
};

//------------------------------------------------------------------------------

template < typename TYPE >
class OptionT : public Option
{
public:

  typedef TYPE value_type;

  OptionT ( const std::string& name, const std::string& desc, TYPE def ) :
      Option(name,DEMANGLED_TYPEID(TYPE), desc, def)
  {
    CFinfo
        << " creating option ["
        << m_name << "] of type ["
        << m_type << "] w default ["
        << def << "] desc ["
        << m_description << "]\n" << CFendl;
  }

};

//------------------------------------------------------------------------------

class OptionList
{
private:

  typedef std::map < std::string , Option::Ptr > OptionStorage_t;

public:

  template < typename OPTION_TYPE >
      Option::Ptr add (const std::string& name, const std::string& description, typename OPTION_TYPE::value_type def )
  {
    cf_assert_desc ( "Class has already option with same name", m_options.find(name) == m_options.end() );
    Option::Ptr opt ( new OPTION_TYPE(name, description, def ) );
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

  Option::Ptr option( const std::string& optname )
  {
    return m_option_list.getOption(optname);
  }

  void configure_opts ( const std::string& pars )
  {

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

    option("OptBool")->attach_processor( boost::bind ( &MyC::config_bool, this ) );
    option("OptInt")->attach_processor ( boost::bind ( &MyC::config_int,  this ) );
    option("OptStr")->attach_processor ( boost::bind ( &MyC::config_str,  this ) );

  };

  void config_bool ()
  {
    boost::any value = option("OptBool")->value();
    bool b = boost::any_cast<bool>(value);
    CFinfo << "config bool [" << Common::StringOps::to_str(b) << "]\n" << CFendl;
  }

  void config_int ()
  {
//    CFinfo << "config int [" << option("OptInt")->value() << "]\n" << CFendl;
  }

  void config_str ()
  {
//    CFinfo << "config str [" << option("OptStr")->value() << "]\n" << CFendl;
  }


  static void defineConfigOptions ( OptionList& options )
  {
    options.add< OptionT<bool> >         ( "OptBool", "bool option"   , false  );
    options.add< OptionT<Uint> >         ( "OptInt",  "int option"    , 10     );
    options.add< OptionT<std::string> >  ( "OptStr",  "string option" , "LOLO" );
  }

private:

//  bool b;
//  Uint i;
//  std::string s;

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

  pm->config_bool();
  pm->config_int();
  pm->config_str();

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


void print_xml_node(rapidxml::xml_node<> *node)
{
  using namespace rapidxml;

//  cout << "Node [" << node->name() << "]";
//  cout << " w value [" << node->value() << "]\n";

  for (xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
  {
//    cout << "++ attribute " << attr->name() << " ";
//    cout << "with value " << attr->value() << "\n";
  }

  for (xml_node<> * itr = node->first_node(); itr; itr = itr->next_sibling() )
  {
    print_xml_node(itr);
  }

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( rapidxml )
{
  using namespace rapidxml;

  std::string text = ( "<debug lolo=\"1\" koko=\"2\" >"
                       "<filename>debug.log</filename>"
                       "<modules NBS=\"3\">"
                       "    <module>Finance</module>"
                       "    <module>Admin</module>"
                       "    <module>HR</module>"
                       "</modules>"
                       "<level>2</level>"
                       "</debug>" );

  xml_document<> doc;    // character type defaults to char

  char* ctext = doc.allocate_string(text.c_str());

  doc.parse< parse_no_data_nodes >(ctext);    // 0 means default parse flags

  print_xml_node(doc.first_node());
//  print_xml_node(doc);

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
