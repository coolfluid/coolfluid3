// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>

#include "Common/Assertions.hpp"
//#include "Common/Foreach.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/PropertyList.hpp"
#include "Common/StringConversion.hpp"
#include "Common/TypeInfo.hpp"
#include "Common/URI.hpp"
//#include "Common/XML/Protocol.hpp"
//#include "Common/OptionArray.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

PropertyList & PropertyList::add_property (const std::string& name,
                                           const boost::any & value )
{
  cf_assert_desc ( "Class has already property with same name",
                   this->store.find(name) == store.end() );
  //
  // cf_assert_desc("The name of property ["+name+"] does not comply with coolfluid standard. "
  //                "It may not contain spaces.",
  //   boost::algorithm::all(name,
  //     boost::algorithm::is_alnum() || boost::algorithm::is_any_of("-_")) );

  store.insert( std::make_pair(name, value ) );
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

const boost::any & PropertyList::property( const std::string& pname) const
{
  PropertyStorage_t::const_iterator itr = store.find(pname);

  if ( itr != store.end() )
    return itr->second;
  else
  {
    std::string msg;
    PropertyStorage_t::const_iterator it = store.begin();

    msg += "Property with name ["+pname+"] not found. Available properties are:\n";

    for (; it!=store.end(); it++)
      msg += "  - " + it->first + "\n";

    throw ValueNotFound(FromHere(), msg);
  }

}

////////////////////////////////////////////////////////////////////////////////

boost::any & PropertyList::property( const std::string& pname)
{
  PropertyStorage_t::iterator itr = store.find(pname);
  if ( itr != store.end() )
    return itr->second;
  else
  {
    std::string msg;
    msg += "Property with name ["+pname+"] not found. Available properties are:\n";
    PropertyStorage_t::iterator it = store.begin();
    for (; it!=store.end(); it++)
      msg += "  - " + it->first + "\n";
    throw ValueNotFound(FromHere(),msg);
  }

}

////////////////////////////////////////////////////////////////////////////////

std::string PropertyList::value_str ( const std::string & pname ) const
{
  const boost::any & value = property( pname ); // throws if prop not found
  std::string value_type = class_name_from_typeinfo( value.type() );

  try
  {
    if (value_type == "bool")
      return to_str(boost::any_cast<bool>(value));
    else if (value_type == "unsigned")
      return to_str(boost::any_cast<Uint>(value));
    else if (value_type == "integer")
      return to_str(boost::any_cast<int>(value));
    else if (value_type == "real")
      return to_str(boost::any_cast<Real>(value));
    else if (value_type == "string")
      return boost::any_cast<std::string>(value);
    else if (value_type == "uri")
      return to_str(boost::any_cast<URI>(value));
    else
      throw ProtocolError(FromHere(),"Property has illegal value type: "+value_type);
  }
  catch(boost::bad_any_cast e)
  {
    throw CastingFailed( FromHere(), "Unable to cast from [" + value_type + "] to string");
  }
}

////////////////////////////////////////////////////////////////////////////////

std::string PropertyList::type( const std::string & pname ) const
{
  return class_name_from_typeinfo( property(pname).type() );
}

////////////////////////////////////////////////////////////////////////////////

//const Option & PropertyList::option( const std::string& pname) const
//{
//  return option(pname).as_option();
//}

//////////////////////////////////////////////////////////////////////////////////

//Option & PropertyList::option( const std::string& pname)
//{
//  return option(pname).as_option();
//}

////////////////////////////////////////////////////////////////////////////////

void PropertyList::erase( const std::string& pname)
{
  PropertyStorage_t::iterator itr = store.find(pname);
  if ( itr != store.end() )
    store.erase(itr);
  else
    throw ValueNotFound(FromHere(), "Property with name [" + pname + "] not found" );
}

////////////////////////////////////////////////////////////////////////////////

boost::any & PropertyList::operator [] (const std::string & pname)
{
  PropertyStorage_t::iterator itr = store.find(pname);

  if ( itr != store.end() )
    return itr->second;
  else
  {
    add_property(pname, boost::any());
    return store[pname];
  }
}

////////////////////////////////////////////////////////////////////////////////

const boost::any & PropertyList::operator [] (const std::string & pname) const
{
  PropertyStorage_t::const_iterator itr = store.find(pname);

  if ( itr != store.end() )
    return itr->second;
  else
  {
    std::string msg;
    msg += "Property with name ["+pname+"] not found. Available properties are:\n";
    PropertyStorage_t::const_iterator it = store.begin();
    for (; it!=store.end(); it++)
      msg += "  - " + it->first + "\n";
    throw ValueNotFound(FromHere(),msg);
  }
}

////////////////////////////////////////////////////////////////////////////////

void PropertyList::configure_property(const std::string& pname, const boost::any& val)
{
  PropertyStorage_t::iterator itr = store.find(pname);
  if (itr == store.end())
  {
    std::string msg;
    msg += "Property with name ["+pname+"] not found. Available properties are:\n";
    if (store.size())
    {
      PropertyStorage_t::iterator it = store.begin();
      for (; it!=store.end(); it++)
        msg += "  - " + it->first + "\n";
    }
    throw ValueNotFound(FromHere(),msg);
  }

  itr->second = val;
}

/////////////////////////////////////////////////////////////////////////////////////
template < typename TYPE >
TYPE PropertyList::value( const std::string & pname) const
{
  const_iterator found_prop = store.find(pname);
  boost::any value;

  if( found_prop == store.end() )
  {
    std::string msg;
    msg += "Property with name ["+pname+"] not found. Available properties are:\n";
    PropertyStorage_t::const_iterator it = store.begin();
    for (; it!=store.end(); it++)
      msg += "  - " + it->first + "\n";
    throw ValueNotFound(FromHere(),msg);
  }

  value = found_prop->second;

  try
  {
    return boost::any_cast< TYPE >( value );
  }
  catch(boost::bad_any_cast& e)
  {
    throw CastingFailed( FromHere(), "Bad boost::any cast from " +
                         class_name_from_typeinfo( value.type() ) +
                         " to " + class_name<TYPE>());
  }
}

/////////////////////////////////////////////////////////////////////////////////////

Common_TEMPLATE template bool PropertyList::value<bool>(const std::string &) const;
Common_TEMPLATE template int PropertyList::value<int>(const std::string &) const;
Common_TEMPLATE template Uint PropertyList::value<Uint>(const std::string &) const;
Common_TEMPLATE template Real PropertyList::value<Real>(const std::string &) const;
Common_TEMPLATE template std::string PropertyList::value<std::string>(const std::string &) const;
Common_TEMPLATE template URI PropertyList::value<URI>(const std::string &) const;

} // Common
} // CF

