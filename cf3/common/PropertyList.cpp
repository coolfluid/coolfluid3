// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>

#include "common/Assertions.hpp"
#include "common/BasicExceptions.hpp"
#include "common/PropertyList.hpp"
#include "common/TypeInfo.hpp"

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////////////

PropertyList & PropertyList::add_property (const std::string& name,
                                           const boost::any & value )
{
  cf3_assert_desc ( "Class has already property with same name",
                   this->store.find(name) == store.end() );

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
  return any_to_str(property(pname));
}

////////////////////////////////////////////////////////////////////////////////

std::string PropertyList::type( const std::string & pname ) const
{
  return class_name_from_typeinfo( property(pname).type() );
}

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
  return store[pname];
}

////////////////////////////////////////////////////////////////////////////////

const boost::any & PropertyList::operator [] (const std::string & pname) const
{
  return property(pname);
}

////////////////////////////////////////////////////////////////////////////////

void PropertyList::configure_property(const std::string& pname, const boost::any& val)
{
  property(pname) = val;
}


} // common
} // cf3

