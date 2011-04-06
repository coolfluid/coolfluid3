// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "rapidxml/rapidxml.hpp"

#include "Common/Assertions.hpp"
#include "Common/BasicExceptions.hpp"

#include "Common/XML/Protocol.hpp"

#include "Common/XML/SignalOptions.hpp"

// makes explicit instantiation for all template functions with a same type
#define TEMPLATE_EXPLICIT_INSTANTIATION(T) \
  Common_TEMPLATE template SignalOptions & SignalOptions::add<T>(const std::string&, const T&, const std::string &, const std::vector<T>&, const std::string&);\
  Common_TEMPLATE template SignalOptions & SignalOptions::add<T>(const std::string&, const std::vector<T>&, const std::string&, const std::string &, const std::vector<T>&);\
  Common_TEMPLATE template T SignalOptions::option<T>(const std::string&) const;\
  Common_TEMPLATE template std::vector<T> SignalOptions::array<T>(const std::string&) const;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace XML {

//////////////////////////////////////////////////////////////////////////////

SignalOptions::SignalOptions( SignalFrame frame )
{
  // note: no need to check if frame is valid, SignalFrame::map() does that
  // for us.

  map = frame.map( Protocol::Tags::key_options() ).main_map;
}

//////////////////////////////////////////////////////////////////////////////

template<typename TYPE>
SignalOptions & SignalOptions::add ( const std::string & name, const TYPE & value,
                                     const std::string & descr,
                                     const std::vector<TYPE> & restr_values,
                                     const std::string & restr_values_delim )
{
  // if there are restricted values:
  // 1. the delimiter can not be empty
  cf_assert( restr_values.empty() || !restr_values_delim.empty() );
  // 2. the value must be present in the restricted list of values
  cf_assert( restr_values.empty() || std::find(restr_values.begin(), restr_values.end(), value) != restr_values.end() );

  if( map.check_entry(name) )
    throw ValueExists(FromHere(), "Option with name [" + name + "] already exists.");

  XmlNode node = map.set_value( name, value, descr );

  // if there are restricted, we add them as an array
  if( !restr_values.empty() )
    Map(node).set_array( Protocol::Tags::key_restricted_values(), restr_values, restr_values_delim );

  return *this;
}

//////////////////////////////////////////////////////////////////////////////

template<typename TYPE>
SignalOptions & SignalOptions::add ( const std::string & name,
                                     const std::vector<TYPE> & value,
                                     const std::string & delimiter,
                                     const std::string & descr,
                                     const std::vector<TYPE> & restr_values )
{
  if( map.check_entry(name) )
    throw ValueExists(FromHere(), "Option with name [" + name + "] already exists.");

  XmlNode node = map.set_array( name, value, delimiter, descr );

  // if there are restricted, we add them as an array
  if( !restr_values.empty() )
    Map(node).set_array( Protocol::Tags::key_restricted_values(), restr_values, delimiter );

  return *this;
}

//////////////////////////////////////////////////////////////////////////////

SignalOptions & SignalOptions::add ( const std::string & name, const URI & value,
                                     const std::string & descr,
                                     const std::vector<URI::Scheme::Type> & sup_schemes,
                                     const std::vector<URI> & restr_values,
                                     const std::string & restr_values_delim)

{
  // if there are restricted values:
  // 1. the delimiter can not be empty
  cf_assert( restr_values.empty() || !restr_values_delim.empty() );
  // 2. the value must be present in the restricted list of values
  cf_assert( restr_values.empty() || std::find(restr_values.begin(), restr_values.end(), value) != restr_values.end() );

  if( map.check_entry(name) )
    throw ValueExists(FromHere(), "Option with name [" + name + "] already exists.");

  XmlNode node = map.set_value( name, value, descr );
  std::string schemes_str;
  std::vector<URI::Scheme::Type>::const_iterator it = sup_schemes.begin();

  // if there are restricted, we add them as an array
  if( !restr_values.empty() )
    Map(node).set_array( Protocol::Tags::key_restricted_values(), restr_values, restr_values_delim );

  // build the allowed scheme string
  for( ; it != sup_schemes.end() ; ++it )
  {
    if( !schemes_str.empty() )
      schemes_str += ',';

    schemes_str += URI::Scheme::Convert::instance().to_str(*it);
  }

  node.set_attribute( Protocol::Tags::attr_uri_schemes(), schemes_str );

  return *this;
}

//////////////////////////////////////////////////////////////////////////////

SignalOptions & SignalOptions::add ( const std::string & name,
                                     const std::vector<URI> & value,
                                     const std::string & delimiter,
                                     const std::string & descr,
                                     const std::vector<URI::Scheme::Type> & sup_schemes,
                                     const std::vector<URI> & restr_values )
{
  if( map.check_entry(name) )
    throw ValueExists(FromHere(), "Option with name [" + name + "] already exists.");

  XmlNode node = map.set_array( name, value, delimiter, descr );
  std::string schemes_str;
  std::vector<URI::Scheme::Type>::const_iterator it = sup_schemes.begin();


  // if there are restricted, we add them as an array
  if( !restr_values.empty() )
    Map(node).set_array( Protocol::Tags::key_restricted_values(), restr_values, delimiter );

  // build the allowed scheme string
  for( ; it != sup_schemes.end() ; ++it )
  {
    if( !schemes_str.empty() )
      schemes_str += ',';

    schemes_str += URI::Scheme::Convert::instance().to_str(*it);
  }

  node.set_attribute( Protocol::Tags::attr_uri_schemes(), schemes_str );

  return *this;
}

//////////////////////////////////////////////////////////////////////////////

template<typename TYPE>
TYPE SignalOptions::option( const std::string & name ) const
{
  return map.get_value<TYPE>( name );
}

//////////////////////////////////////////////////////////////////////////////

template<typename TYPE>
std::vector<TYPE> SignalOptions::array( const std::string & name ) const
{
  return map.get_array<TYPE>( name );
}

//////////////////////////////////////////////////////////////////////////////

SignalOptions & SignalOptions::remove ( const std::string & name )
{
  if( !name.empty() )
  {
    XmlNode value_node = map.find_value( name );

    // if the node was found, we remove it
    if( value_node.is_valid() )
      value_node.content->parent()->remove_node( value_node.content );
  }
}

//////////////////////////////////////////////////////////////////////////////

bool SignalOptions::exists ( const std::string & name ) const
{
  return map.check_entry(name);
}

/////////////////////////////////////////////////////////////////////////////////

/// explicit instantiation to avoid missing symbols on certain compilers
TEMPLATE_EXPLICIT_INSTANTIATION( bool );
TEMPLATE_EXPLICIT_INSTANTIATION( int );
TEMPLATE_EXPLICIT_INSTANTIATION( std::string );
TEMPLATE_EXPLICIT_INSTANTIATION( CF::Uint );
TEMPLATE_EXPLICIT_INSTANTIATION( CF::Real );

Common_TEMPLATE template URI SignalOptions::option<URI>(const std::string&) const;
Common_TEMPLATE template std::vector<URI> SignalOptions::array<URI>(const std::string&) const;

//////////////////////////////////////////////////////////////////////////////

} // XML
} // Common
} // CF


/////////////////////////////////////////////////////////////////////////////////

#undef TEMPLATE_EXPLICIT_INSTANTIATION
