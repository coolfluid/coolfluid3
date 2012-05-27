// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file PropertyList.hpp
/// @note This header gets included indirectly in common/Component.hpp
///       It should be as lean as possible!

#ifndef cf3_common_PropertyList_hpp
#define cf3_common_PropertyList_hpp

/////////////////////////////////////////////////////////////////////////////////////

#include <boost/any.hpp>

#include "common/BoostAnyConversion.hpp"
#include "common/CommonAPI.hpp"

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////////////

  /// Class defines a list of options to be used in the ConfigObject class
  /// @author Tiago Quintino
  class Common_API PropertyList
  {

  public:

    /// type to store the options per name
    typedef std::map < std::string , boost::any > PropertyStorage_t;

    typedef PropertyStorage_t::iterator       iterator;
    typedef PropertyStorage_t::const_iterator const_iterator;

  public:

    /// adds a property to the list
    PropertyList & add (const std::string& name, const boost::any & value);

    /// get a const property from the list
    const boost::any& property( const std::string& pname ) const;
    /// get a property from the list
    boost::any& property( const std::string& pname );

    /// contant access operator to properties
    const boost::any& operator [] (const std::string & pname) const;
    /// access operator to properties
    boost::any& operator [] (const std::string & pname);

    /// Configure one option, and trigger its actions
    /// @param [in] optname  The option name
    /// @param [in] val      The new value assigned to the option
    void set(const std::string& pname, const boost::any& val);

    /// @brief Configure an option on this class from a human readable string.
    ///
    /// The string provides the configuration in one of the following formats:
    /// - var_name:type=value
    /// - var_name:array[type]=val1,val2
    void set(const std::string& arg);

    /// check that a property with the name exists
    /// @param prop_name the property name
    bool check ( const std::string& prop_name ) const
    {
      return store.find(prop_name) != store.end();
    }

    /// erases a property
    /// @param prop_name the property name
    void erase (const std::string & pname);

    /// @returns the value of the option cast to TYPE
    template < typename TYPE >
    TYPE value( const std::string & pname) const
    {
      return any_to_value<TYPE>(property(pname));
    }

    std::string value_str ( const std::string & pname ) const;

    std::string type( const std::string & pname ) const;

    iterator begin() { return store.begin(); }

    iterator end()  { return store.end(); }

    const_iterator begin() const { return store.begin(); }

    const_iterator end() const  { return store.end(); }

  public:

    /// storage of options
    PropertyStorage_t store;

  }; // class PropertyList

/////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_PropertyList_hpp
