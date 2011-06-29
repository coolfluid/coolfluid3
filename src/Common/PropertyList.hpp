// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_PropertyList_hpp
#define CF_Common_PropertyList_hpp

/////////////////////////////////////////////////////////////////////////////////////

//#include "Common/Option.hpp"

#include <boost/any.hpp>

#include "Common/CommonAPI.hpp"

namespace CF {
namespace Common {

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
    PropertyList & add_property (const std::string& name, const boost::any & value);

//    /// adds an option to the list
//    template < typename OPTION_TYPE >
//    Option::Ptr add_option (const std::string& name,
//                            const std::string& description,
//                            const typename OPTION_TYPE::value_type& def=typename OPTION_TYPE::value_type() )
//    {
//      cf_assert_desc ( "Class has already property with same name",
//                       this->store.find(name) == store.end() );
//      Option::Ptr opt ( new OPTION_TYPE(name, description, def) );
////      store.insert( std::make_pair(name, opt ) );
//      return opt;
//    }

//    template < typename OPTION_TYPE >
//    Option::Ptr add_option (const std::string& name,
//                            const std::string& pretty_name,
//                            const std::string& description,
//                            const typename OPTION_TYPE::value_type& def)
//    {
//      cf_assert_desc ( "Class has already property with same name",
//                       this->store.find(name) == store.end() );
//      Option::Ptr opt ( new OPTION_TYPE(name, pretty_name, description, def) );
////      store.insert( std::make_pair(name, opt ) );
//      return opt;
//    }

//    /// adds an option to the list
//    template < typename OPTION_TYPE >
//    Option::Ptr add_option (boost::shared_ptr<OPTION_TYPE> option)
//    {
//      cf_assert_desc ( "Class has already property with same name",
//                       this->store.find(option->name()) == store.end() );
//      Option::Ptr opt = boost::dynamic_pointer_cast<Option>(option);
////      store.insert( std::make_pair(option->name(), opt ) );
//      return opt;
//    }

    /// sets a link to the option
//    template < typename TYPE >
//        void link_to_parameter ( const std::string& pname, TYPE* par )
//    {
//      cf_assert( check(pname) );
//      store[pname]->as_option().link_to(par);
//    }

    /// get a const property from the list
    const boost::any& property( const std::string& pname ) const;
    /// get a property from the list
    boost::any& property( const std::string& pname );

//    /// get a constant option from the list
//    const Option& option( const std::string& pname ) const;
//    /// get an option from the list
//    Option& option( const std::string& pname );

    /// contant access operator to properties
    const boost::any& operator [] (const std::string & pname) const;
    /// access operator to properties
    boost::any& operator [] (const std::string & pname);

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

    /// @returns the value of the option cast to TYPE
    template < typename TYPE >
    TYPE value( const std::string & pname) const;

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

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_PropertyList_hpp
