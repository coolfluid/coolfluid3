// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>
#include "Common/BasicExceptions.hpp"

#include "Common/PropertyList.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

Property::Ptr PropertyList::add_property (const std::string& name,
                                          const boost::any & value )
{
  cf_assert_desc ( "Class has already property with same name",
                   this->store.find(name) == store.end() );
  // 
  // cf_assert_desc("The name of property ["+name+"] does not comply with coolfluid standard. "
  //                "It may not contain spaces.",
  //   boost::algorithm::all(name,
  //     boost::algorithm::is_alnum() || boost::algorithm::is_any_of("-_")) );

  Property::Ptr prop ( new Property(value) );
  store.insert( std::make_pair(name, prop ) );
  return prop;
}

////////////////////////////////////////////////////////////////////////////////

const Property & PropertyList::property( const std::string& pname) const
{
  PropertyStorage_t::const_iterator itr = store.find(pname);
  if ( itr != store.end() )
    return *itr->second.get();
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

Property & PropertyList::property( const std::string& pname)
{
  PropertyStorage_t::iterator itr = store.find(pname);
  if ( itr != store.end() )
    return *itr->second.get();
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

const Option & PropertyList::option( const std::string& pname) const
{
  return property(pname).as_option();
}

////////////////////////////////////////////////////////////////////////////////

Option & PropertyList::option( const std::string& pname)
{
  return property(pname).as_option();
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

Property & PropertyList::operator [] (const std::string & pname)
{
  Property::Ptr prop;
  PropertyStorage_t::iterator itr = store.find(pname);

  if ( itr != store.end() )
    prop = itr->second;
  else
    prop = add_property(pname, boost::any());

  return *prop.get();
}

////////////////////////////////////////////////////////////////////////////////

const Property & PropertyList::operator [] (const std::string & pname) const
{
  Property::ConstPtr prop;
  PropertyStorage_t::const_iterator itr = store.find(pname);
  
  if ( itr != store.end() )
    prop = itr->second;
  else
  {
    std::string msg;
    msg += "Property with name ["+pname+"] not found. Available properties are:\n";
    PropertyStorage_t::const_iterator it = store.begin();
    for (; it!=store.end(); it++)
      msg += "  - " + it->first + "\n";
    throw ValueNotFound(FromHere(),msg);
  }
  return *prop.get();
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
  Property::Ptr prop = itr->second;
  
  // update the value and trigger its actions (if it is an option)
  if(prop->is_option())
  {
    prop->as_option().change_value(val);
  }
  else
  {
    prop->change_value(val);
  }
}
  

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

