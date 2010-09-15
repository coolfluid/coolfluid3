// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_ConfigObject_hpp
#define CF_Common_ConfigObject_hpp

/////////////////////////////////////////////////////////////////////////////////////

#include <boost/foreach.hpp>
#include "Common/Option.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

  /// Class defines a list of options to be used in the ConfigObject class
  /// @author Tiago Quintino
  class Common_API OptionList {

  public:

    /// type to store the options per name
    typedef std::map < std::string , Option::Ptr > OptionStorage_t;

  public:

    /// add an option to the list
    template < typename OPTION_TYPE >
    Option::Ptr add (const std::string& name, const std::string& description, const typename OPTION_TYPE::value_type& def )
    {
      cf_assert_desc ( "Class has already option with same name",
                       this->m_options.find(name) == m_options.end() );
      Option::Ptr opt ( new OPTION_TYPE(name, description, def ) );
      m_options.insert( std::make_pair(name, opt ) );
      return opt;
    }

    /// get and option from the list
    Option::Ptr getOption( const std::string& optname );
    
    /// Configure one option, and trigger its actions
    /// @param [in] optname  The option name
    /// @param [in] val      The new value assigned to the option
    void configure_option(const std::string& optname, const boost::any& val)
    {
      getOption(optname)->change_value(val); // update the value and trigger its actions
    }

  public:

    /// storage of options
    OptionStorage_t m_options;

  }; // OptionList

/////////////////////////////////////////////////////////////////////////////////////

  /// Class defines a object that has options that can be dynamically
  /// configured by the end-user at run-time.
  /// @author Tiago Quintino
  class Common_API ConfigObject  {

  public:

    /// Sets the config options by calling the defineConfigOptions
    /// This will add nested names to the options as opposed to addOptionsTo
    /// @param prt pass the this pointer to help identify callee CLASS type
    template <typename CLASS>
        void addConfigOptionsTo()
    {
      CLASS::defineConfigOptions(m_option_list);
    }

    /// configures all the options on this class
    void configure ( XmlNode& node );

    /// sets a link to the option
    template < typename TYPE >
        void link_to_parameter ( const std::string& optname, TYPE* par )
    {
        option(optname)->link_to(par);
    }

    /// get the pointer to the option
    Option::Ptr option( const std::string& optname );
    
    /// Configure one option, and trigger its actions
    /// @param [in] optname  The option name
    /// @param [in] val      The new value assigned to the option    
    void configure_option(const std::string& optname, const boost::any& val)
    {
      m_option_list.configure_option(optname,val);
    }


  protected:

    /// storage of the option list
    OptionList m_option_list;

  }; // ConfigObject

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_ConfigObject_hpp
