// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file OptionList.hpp

#ifndef cf3_common_OptionList_hpp
#define cf3_common_OptionList_hpp

/////////////////////////////////////////////////////////////////////////////////////

#include "common/CF.hpp"
#include "common/Assertions.hpp"
#include "common/Option.hpp"
#include "common/OptionListDetail.hpp"

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////////////

/// Class defines a list of options to be used in the ConfigObject class
/// @author Tiago Quintino
class Common_API OptionList
{

public:

  /// type to store the options per name
  typedef std::map < std::string , boost::shared_ptr<Option> > OptionStorage_t;

  typedef OptionStorage_t::iterator       iterator;
  typedef OptionStorage_t::const_iterator const_iterator;

public:
  /// Create an option and add it to the list
  /// @param T Template parameter specifying the type held by the option.
  /// @param name Name of the option
  /// @param default_value Default value for the option
  /// @pre An option with the same name does not exist
  /// @return A reference to the created option
  template<typename T>
  typename SelectOptionType<T>::type& add (const std::string& name, const T& default_value = T())
  {
    cf3_assert_desc ( "Class already has an option with same name",
                      this->store.find(name) == store.end() );

    typedef typename SelectOptionType<T>::type OptionType;
    boost::shared_ptr<OptionType> opt ( new OptionType(name, default_value) );
    store.insert( std::make_pair(name, opt ) );
    return *opt;
  }

  /// adds an externally created option to the list
  /// @pre An option with the same name does not exist
  /// @return A reference to the added option
  template < typename OPTION_TYPE >
  OPTION_TYPE& add (const boost::shared_ptr<OPTION_TYPE>& option)
  {
    cf3_assert_desc ( "Class has already property with name " + option->name(),
                      this->store.find(option->name()) == store.end() );

    store.insert( std::make_pair(option->name(), option ) );
    return *option;
  }

  /// sets a link to the option
  template < typename TYPE >
      void link_to_parameter ( const std::string& pname, TYPE* par )
  {
    cf3_assert( check(pname) );
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
  void set(const std::string& pname, const boost::any& val);

  /// @brief Get the value of the option with given name
  /// @param [in] opt_name  The option name
  /// @return option value with correct type
  template < typename TYPE >
    const TYPE value ( const std::string& opt_name ) const
  {
    return option(opt_name).value<TYPE>();
  }

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
  std::string list_options() const;

  /// Parses the strings to options.

  /// If an option already exists in the list, its value is modified with the
  /// new one. If it does not exist yet, it is added.

  /// @brief Configure an option on this class from a human readable string.
  ///
  /// The string provides the configuration in one of the following formats:
  /// - var_name:type=value
  /// - var_name:array[type]=val1,val2
  void set (const std::string& arg);

  /// @brief Configure an option on this class from a list of human readable strings
  ///
  /// The strings provide the configuration in one of the following formats:
  /// - var_name:type=value
  /// - var_name:array[type]=val1,val2
  void set( const std::vector<std::string> & args );

public:

  /// storage of options
  OptionStorage_t store;

}; // class OptionList

/////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_OptionList_hpp
