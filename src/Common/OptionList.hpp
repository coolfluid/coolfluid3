// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_OptionList_hpp
#define CF_Common_OptionList_hpp

/////////////////////////////////////////////////////////////////////////////////////

//#include <boost/type_traits/is_base_of.hpp>

#include "Common/Option.hpp"

namespace CF {
namespace Common {

//class Component;
//template<typename T> class OptionComponent;
//template<typename T> class OptionT;
//template<typename T> struct SelectValueType;

/////////////////////////////////////////////////////////////////////////////////////

  /// Class defines a list of options to be used in the ConfigObject class
  /// @author Tiago Quintino
  class Common_API OptionList
  {

  public:

    /// type to store the options per name
    typedef std::map < std::string , Option::Ptr > OptionStorage_t;

    typedef OptionStorage_t::iterator       iterator;
    typedef OptionStorage_t::const_iterator const_iterator;

//    /// Helper to choose the appropriate return type
//    template<typename T>
//    struct SelectOptionType
//    {
//      typedef typename boost::mpl::if_
//      <
//        boost::is_base_of<Component, T>, // If T is a component...
//        OptionComponent<T>,              // we return an OptionComponent
//        OptionT<T>                       // otherwise we have a generic option
//      >::type type;
//    };

  public:

    /// adds an option to the list
    template < typename OPTION_TYPE >
    Option::Ptr add_option (const std::string& name,
                            const typename OPTION_TYPE::value_type& def=typename OPTION_TYPE::value_type() )
    {
      cf_assert_desc ( "Class has already property with same name",
                       this->store.find(name) == store.end() );
      Option::Ptr opt ( new OPTION_TYPE(name, def) );
      store.insert( std::make_pair(name, opt ) );
      return opt;
    }

    /// adds an option to the list
    template < typename OPTION_TYPE >
    Option::Ptr add_option (boost::shared_ptr<OPTION_TYPE> option)
    {
      cf_assert_desc ( "Class has already property with same name",
                       this->store.find(option->name()) == store.end() );
      Option::Ptr opt = boost::dynamic_pointer_cast<Option>(option);
      store.insert( std::make_pair(option->name(), opt ) );
      return opt;
    }


//    /// Add option, variant with default parameter
//    template<typename T>
//    typename SelectOptionType<T>::type::Ptr add_opt(const std::string& name, const typename SelectOptionType<T>::type::value_type & default_value = typename SelectValueType<T>::type());

    /// sets a link to the option
    template < typename TYPE >
        void link_to_parameter ( const std::string& pname, TYPE* par )
    {
      cf_assert( check(pname) );
      store[pname]->link_to(par);
    }

    /// get a constant option from the list
    const Option& option( const std::string& pname ) const;
    /// get an option from the list
    Option& option( const std::string& pname );

    /// contant access operator to properties
    const Option& operator [] (const std::string & pname) const;
    /// access operator to properties
    Option& operator [] (const std::string & pname);

    /// Configure one option, and trigger its actions
    /// @param [in] optname  The option name
    /// @param [in] val      The new value assigned to the option
    void configure_option(const std::string& pname, const boost::any& val);

    /// check that a option with the name exists
    /// @param opt_name the property name
    bool check ( const std::string& opt_name ) const
    {
      return store.find(opt_name) != store.end();
    }

    /// erases an option from the list
    /// @param name the option name to erase
    void erase (const std::string & name);

    iterator begin() { return store.begin(); }

    iterator end()  { return store.end(); }

    const_iterator begin() const { return store.begin(); }

    const_iterator end() const  { return store.end(); }

    /// list the options as a string
    std::string list_options();

  public:

    /// storage of options
    OptionStorage_t store;

  }; // class OptionList

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OptionList_hpp
