#ifndef CF_Common_ConfigObject_hpp
#define CF_Common_ConfigObject_hpp

/////////////////////////////////////////////////////////////////////////////////////

#include "Common/Option.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

  /// Class defines a list of options to be used in the ConfigObject class
  /// @author Tiago Quintino
  class OptionList {

  public:

    /// type to store the options per name
    typedef std::map < std::string , Option::Ptr > OptionStorage_t;

  public:

    /// add an option to the list
    template < typename OPTION_TYPE >
    Option::Ptr add (const std::string& name, const std::string& description, const typename OPTION_TYPE::value_type& def )
    {
      cf_assert_desc ( "Class has already option with same name", this->m_options.find(name) == m_options.end() );
      Option::Ptr opt ( new OPTION_TYPE(name, description, def ) );
      m_options.insert( std::make_pair(name, opt ) );
      return opt;
    }

    /// get and option from the list
    Option::Ptr getOption( const std::string& optname );

  public:

    /// storage of options
    OptionStorage_t m_options;

  }; // OptionList

/////////////////////////////////////////////////////////////////////////////////////

  /// Class defines a object that has options that can be dynamically
  /// configured by the end-user at run-time.
  /// @author Tiago Quintino
  class ConfigObject  {

  public:

    /// Sets the config options by calling the defineConfigOptions
    /// This will add nested names to the options as opposed to addOptionsTo
    /// @param prt should be passed with the this pointer to help identify callee the CLASS type
    template <typename CLASS>
        void addConfigOptionsTo(const CLASS* ptr)
    {
      CLASS::defineConfigOptions(m_option_list);
    }

    /// configures all the options on this class
    void configure ( rapidxml::xml_node<> *node );

  protected:

    /// get the pointer to the option
    Option::Ptr option( const std::string& optname );

  protected:

    /// storage of the option list
    OptionList m_option_list;

  }; // ConfigObject

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_ConfigObject_hpp
