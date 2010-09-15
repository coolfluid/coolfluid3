// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_PropertyList_hpp
#define CF_Common_PropertyList_hpp

/////////////////////////////////////////////////////////////////////////////////////

#include "Common/Property.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

  /// Class defines a list of options to be used in the ConfigObject class
  /// @author Tiago Quintino
  class Common_API PropertyList {

  public:

    /// type to store the options per name
    typedef std::map < std::string , Property::Ptr > PropertyStorage_t;

  public:

    /// adds a property to the list
    template < typename OPTION_TYPE >
    Property::Ptr add_property (const std::string& name,
                                const typename OPTION_TYPE::value_type& value )
    {
      return add< OPTION_TYPE >(name, "", value, false);
    }

    /// adds an option to the list
    template < typename OPTION_TYPE >
    Property::Ptr add_option (const std::string& name,
                              const std::string& description,
                              const typename OPTION_TYPE::value_type& def )
    {
      return add< OPTION_TYPE >(name, description, def, true);
    }

    /// get a property from the list
    Property::Ptr getProperty( const std::string& optname );

    /// Configure one option, and trigger its actions
    /// @param [in] optname  The option name
    /// @param [in] val      The new value assigned to the option
    void configure_property(const std::string& optname, const boost::any& val)
    {
      getProperty(optname)->change_value(val); // update the value and trigger its actions
    }

    /// check that a property with the name exists
    /// @param prop_name the property name
    bool check ( const std::string& prop_name ) const
    {
      return m_properties.find(prop_name) != m_properties.end();
    }

    /// erases a property
    /// @param prop_name the property name
    void erase(const std::string & pname);

  public:

    /// storage of options
    PropertyStorage_t m_properties;

  private:

    /// adds a property to the list
    template < typename OPTION_TYPE >
    Property::Ptr add (const std::string& name, const std::string& description,
                       const typename OPTION_TYPE::value_type& def, bool is_option)
    {
      cf_assert_desc ( "Class has already property with same name",
                       this->m_properties.find(name) == m_properties.end() );
      Property::Ptr opt ( new OPTION_TYPE(name, description, def, is_option) );
      m_properties.insert( std::make_pair(name, opt ) );
      return opt;
    }


  }; // class PropertyList

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_PropertyList_hpp
