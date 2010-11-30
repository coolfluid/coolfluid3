// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_PropertyList_hpp
#define CF_Common_PropertyList_hpp

/////////////////////////////////////////////////////////////////////////////////////

#include "Common/Property.hpp"
#include "Common/Option.hpp"

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
    Property::Ptr add_property (const std::string& name, const boost::any & value);

    /// adds an option to the list
    template < typename OPTION_TYPE >
    Option::Ptr add_option (const std::string& name,
                              const std::string& description,
                              const typename OPTION_TYPE::value_type& def )
    {
      cf_assert_desc ( "Class has already property with same name",
                       this->store.find(name) == store.end() );
      Option::Ptr opt ( new OPTION_TYPE(name, description, def) );
      store.insert( std::make_pair(name, opt ) );
      return opt;
    }

    /// sets a link to the option
    template < typename TYPE >
        void link_to_parameter ( const std::string& pname, TYPE* par )
    {
      cf_assert( check(pname) );
      store[pname]->as_option().link_to(par);
    }

    /// get a property from the list
    const Property& property( const std::string& pname ) const;

    /// get an option from the list
    const Option& option( const std::string& pname ) const;

    Property& operator [] (const std::string & pname);
		
    const Property& operator [] (const std::string & pname) const;

    /// Configure one option, and trigger its actions
    /// @param [in] optname  The option name
    /// @param [in] val      The new value assigned to the option
    void configure_property(const std::string& pname, const boost::any& val);
		
    /// check that a property with the name exists
    /// @param prop_name the property name
    bool check ( const std::string& prop_name ) const
    {
      return store.find(prop_name) != store.end();
    }

    /// erases a property
    /// @param prop_name the property name
    void erase (const std::string & pname);

  public:

    /// storage of options
    PropertyStorage_t store;

  }; // class PropertyList

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_PropertyList_hpp
