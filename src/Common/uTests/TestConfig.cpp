#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/foreach.hpp>

#include <boost/property_tree/detail/rapidxml.hpp>
#include <boost/any.hpp>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>



#include "Common/CF.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/Log.hpp"
#include "Common/Component.hpp"

using namespace std;
using namespace boost;

using namespace CF;
using namespace CF::Common;


/// @todo
///   * option of pointer to base class init from self regist
///   * option for pointer to Component
///   * vector of components
///   * modify DynamicObject class - signals with XML
///   * option sets with own processors
///   * option for paths ( file and dirs )
///   * option for component path
///   * break into files
///       - Configurable ( Configurable, OptionList )
///       - Option ( Option, OptionT )
///
/// How to:
///   * how to define processors statically?
///   * how to define the validations statically??
///   * components inform GUI of
///      * their signals
///      * hide signals from GUI in advanced mode
///      * inform of XML parameters for each signal
///
/// Done:
///   * option of vector of T
///   * configure values from XMLNode
///   * access configured values

////////////////////////////////////////////////////////////////////////////////

class Option
{
public:

  typedef boost::shared_ptr<Option> Ptr;
  typedef boost::function< void()> Processor_t;
  typedef std::vector< Processor_t > ProcStorage_t;

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

  virtual ~Option () {}

  /// Assume that the xml node passed is correct for this option
  virtual void change_value ( rapidxml::xml_node<> *node ) = 0;

  void configure_option ( rapidxml::xml_node<> *node )
  {
    this->change_value(node); // update the value

    // call all process functors
    BOOST_FOREACH( Option::Processor_t& process, m_processors )
        process();
  }

  void attach_processor ( Processor_t proc ) { m_processors.push_back(proc); }

  // accessor functions

  std::string name() const { return m_name; }
  std::string type() const { return m_name; }
  std::string description() const { return m_description; }

  boost::any value() const { return m_value; }
  boost::any def() const { return m_default; }

  template < typename TYPE >
      TYPE value() const { return boost::any_cast<TYPE>(m_value); }
  template < typename TYPE >
      TYPE def() const { return boost::any_cast<TYPE>(m_default); }

  template < typename TYPE >
  void put_value( TYPE& value ) const { value = boost::any_cast<TYPE>(m_value); }
  template < typename TYPE >
  void put_def( TYPE& def ) const { def = boost::any_cast<TYPE>(m_default); }

protected:

  boost::any m_value;
  const boost::any m_default;

  std::string m_name;
  std::string m_type;
  std::string m_description;

  ProcStorage_t m_processors;
};

//------------------------------------------------------------------------------

template < typename TYPE >
struct ConvertValue
{
  static TYPE convert ( char * str )
  {
    CFinfo << "converting string to POD\n" << CFendl;
    std::string ss (str);
    return StringOps::from_str< TYPE >(ss);
  }
};

template < typename TYPE >
struct ConvertValue< std::vector<TYPE> >
{
  static std::vector<TYPE> convert ( char * str )
  {
    CFinfo << "converting string to vector\n" << CFendl;

    std::vector < TYPE > vvalues; // start with clean vector

    std::string ss (str);
    boost::tokenizer<> tok (ss);
    for(boost::tokenizer<>::iterator elem = tok.begin(); elem != tok.end(); ++elem)
    {
      vvalues.push_back( StringOps::from_str< TYPE >(*elem) );
    }

    return vvalues; // assign to m_value (replaces old values)
  }
};

//------------------------------------------------------------------------------

template < typename TYPE >
class OptionT : public Option
{
public:

  typedef TYPE value_type;

  OptionT ( const std::string& name, const std::string& desc, value_type def ) :
      Option(name,DEMANGLED_TYPEID(value_type), desc, def)
  {
    CFinfo
        << " creating option ["
        << m_name << "] of type ["
        << m_type << "] w default ["
//        << def << "] desc ["
        << m_description << "]\n" << CFendl;
  }

  virtual void change_value ( rapidxml::xml_node<> *node )
  {
    TYPE vt = ConvertValue<TYPE>::convert( node->value() );
    m_value = vt;
  }

};

//------------------------------------------------------------------------------

class OptionComponent : public Option
{
public:

  typedef std::string value_type;

  OptionComponent ( const std::string& name, const std::string& desc, const std::string& def_name ) :
      Option(name, Component::getClassName(), desc, def_name )
  {
    CFinfo
        << " creating option ["
        << m_name << "] of type ["
        << m_type << "] w default ["
        << def_name << "] desc ["
        << m_description << "]\n" << CFendl;
  }

  virtual void change_value ( rapidxml::xml_node<> *node )
  {
    std::string keyname = node->value();
    m_value = keyname;

    m_component.reset(); // delete previous pointee

    Common::SafePtr< Component::PROVIDER > prov =
        Factory< Component >::getInstance().getProvider( keyname );

    m_component = prov->create( keyname );
    // for the moment we repeat the keyname for the actual component name
    // later we will create subnodes in the xml,
    //  * one for the concrete type
    //  * one for the name

  }

protected:

  /// storage of the component pointer
  Component::Ptr m_component;

};


//------------------------------------------------------------------------------

class OptionList
{

public:

  typedef std::map < std::string , Option::Ptr > OptionStorage_t;

public:

  template < typename OPTION_TYPE >
      Option::Ptr add (const std::string& name, const std::string& description, const typename OPTION_TYPE::value_type& def )
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

  void configure ( rapidxml::xml_node<> *node )
  {
    using namespace rapidxml;
    OptionList::OptionStorage_t& options = m_option_list.m_options;

    // loop on the child nodes which should be option configurations
    for (xml_node<>* itr = node->first_node(); itr; itr = itr->next_sibling() )
    {
      OptionList::OptionStorage_t::iterator opt = options.find( itr->name() );
      if (opt != options.end())
        opt->second->configure_option(itr);
    }
  }

protected:

  Option::Ptr option( const std::string& optname )
  {
    return m_option_list.getOption(optname);
  }

protected:

  OptionList m_option_list;

};


//------------------------------------------------------------------------------

class MyC : public ConfigObject
{
  public:

  MyC ()
  {
    addConfigOptionsTo(this);

    option("OptBool")->attach_processor( boost::bind ( &MyC::config_bool,  this ) );
    option("OptInt")->attach_processor ( boost::bind ( &MyC::config_int,   this ) );
    option("OptStr")->attach_processor ( boost::bind ( &MyC::config_str,   this ) );
    option("VecInt")->attach_processor ( boost::bind ( &MyC::config_vecint,this ) );

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

  static void defineConfigOptions ( OptionList& options )
  {

    // POD's (plain old data)
    options.add< OptionT<bool> >            ( "OptBool", "bool option"   , false  );
    options.add< OptionT<Uint> >            ( "OptInt",  "int option"    , 10     );
    options.add< OptionT<std::string> >     ( "OptStr",  "string option" , "LOLO" );

    // vector of POD's
    std::vector<int> def;  // def += 1,2,3,4,5,6,7,8,9;
    def.push_back(3);
    def.push_back(5);
    def.push_back(7);
    options.add< OptionT< std::vector<int> > >     ( "VecInt",  "vector ints option" , def );

    // option for componets
    options.add< OptionComponent >     ( "Comp",  "component option" , "CLink" );
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

void print_xmlnode( XMLNode node )
{
//  cout << "Node [" << node.getName() << "]\n";

  for ( int i = 0; i < node.nText(); ++i)
  {
//    cout << " w text [" << node.getText() << "]\n";
  }

  for ( int i = 0; i < node.nAttribute(); ++i)
  {
    XMLAttribute attr = node.getAttribute(i);
//    cout << "++ attribute [" << attr.lpszName << "] ";
//    cout << "with value [" << attr.lpszValue << "]\n";
  }

  for ( int i = 0; i < node.nChildNode(); ++i)
  {
    print_xmlnode(node.getChildNode(i));
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

BOOST_AUTO_TEST_CASE( xmlnode )
{
  std::string text = ( "<debug lolo=\"1\" koko=\"2\" >"
                       "<filename>debug.log</filename>"
                       "<modules NBS=\"3\">"
                       "    <module>Finance</module>"
                       "    <module>Admin</module>"
                       "    <module>HR</module>"
                       "</modules>"
                       "<level>2</level>"
                       "</debug>" );

  XMLNode root_node = XMLNode::parseString(text.c_str());

  print_xmlnode(root_node);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
