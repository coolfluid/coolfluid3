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
  Common_TEMPLATE template SignalOptions & SignalOptions::add<T>(const std::string&, const T&, const std::string &);\
  Common_TEMPLATE template SignalOptions & SignalOptions::add<T>(const std::string&, const std::vector<T>&, const std::string&, const std::string &);

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
                                     const std::string & descr )
{
  if( map.check_entry(name) )
    throw ValueExists(FromHere(), "Option with name [" + name + "] already exists.");

  map.set_value( name, value, descr );

  return *this;
}

//////////////////////////////////////////////////////////////////////////////

template<typename TYPE>
SignalOptions & SignalOptions::add ( const std::string & name,
                                     const std::vector<TYPE> & value,
                                     const std::string & delimiter,
                                     const std::string & descr )
{
  if( map.check_entry(name) )
    throw ValueExists(FromHere(), "Option with name [" + name + "] already exists.");

  map.set_array( name, value, delimiter, descr );

  return *this;
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
TEMPLATE_EXPLICIT_INSTANTIATION( CF::Common::URI );

//////////////////////////////////////////////////////////////////////////////

} // XML
} // Common
} // CF


/////////////////////////////////////////////////////////////////////////////////

#undef TEMPLATE_EXPLICIT_INSTANTIATION
