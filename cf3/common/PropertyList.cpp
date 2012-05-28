// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include "common/Foreach.hpp"

#include "common/Assertions.hpp"
#include "common/BasicExceptions.hpp"
#include "common/PropertyList.hpp"
#include "common/TypeInfo.hpp"
#include "common/StringConversion.hpp"
#include "common/URI.hpp"

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////////////

PropertyList & PropertyList::add (const std::string& name,
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

void PropertyList::set(const std::string& pname, const boost::any& val)
{
  property(pname) = val;
}

////////////////////////////////////////////////////////////////////////////////

void PropertyList::set(const std::string& arg)
{
  // extract:   variable_name:type=value   or   variable_name:array[type]=value1,value2
 boost::regex expression(  "([[:word:]]+)(\\:([[:word:]]+)(\\[([[:word:]]+)\\])?=(.*))?"  );
 boost::match_results<std::string::const_iterator> what;

 std::string name;
 std::string type;
 std::string subtype; // in case of array<type>
 std::string value;

 if (regex_search(arg,what,expression))
 {
   name=what[1];
   type=what[3];
   subtype=what[5];
   value=what[6];
   // CFinfo << name << ":" << type << (subtype.empty() ? std::string() : std::string("["+subtype+"]"))  << "=" << value << CFendl;

   if      (type == "bool")
     store[name]=from_str<bool>(value);
   else if (type == "unsigned")
     store[name]=from_str<Uint>(value);
   else if (type == "integer")
     store[name]=from_str<int>(value);
   else if (type == "real")
     store[name]=from_str<Real>(value);
   else if (type == "string")
     store[name]=value;
   else if (type == "uri")
     store[name]=from_str<URI>(value);
   else if (type == "array")
   {
     std::vector<std::string> array;

     // the strings could have comma's inside, brackets, etc...
     Uint in_brackets(0);
     std::string::iterator first = value.begin();
     std::string::iterator it = first;
     for ( ; it!=value.end(); ++it)
     {
       if (*it == '(') // opening bracket
         ++in_brackets;
       else if (*it == ')') // closing bracket
         --in_brackets;
       else if (*it == ',' && in_brackets == 0)
       {
         array.push_back(std::string(first,it));
         boost::algorithm::trim(array.back());
         first = it+1;
       }
     }
     array.push_back(std::string(first,it));
     boost::algorithm::trim(array.back());

     if (subtype == "bool")
     {
       std::vector<bool> vec; vec.reserve(array.size());
       boost_foreach(const std::string& str_val,array)
           vec.push_back(from_str<bool>(str_val));
       store[name]=vec;
     }
     else if (subtype == "unsigned")
     {
       std::vector<Uint> vec; vec.reserve(array.size());
       boost_foreach(const std::string& str_val,array)
           vec.push_back(from_str<Uint>(str_val));
       store[name]=vec;
     }
     else if (subtype == "integer")
     {
       std::vector<int> vec; vec.reserve(array.size());
       boost_foreach(const std::string& str_val,array)
           vec.push_back(from_str<int>(str_val));
       store[name]=vec;
     }
     else if (subtype == "real")
     {
       std::vector<Real> vec; vec.reserve(array.size());
       boost_foreach(const std::string& str_val,array)
           vec.push_back(from_str<Real>(str_val));
       store[name]=vec;
     }
     else if (subtype == "string")
     {
       store[name]=array;
     }
     else if (subtype == "uri")
     {
       std::vector<URI> vec; vec.reserve(array.size());
       boost_foreach(const std::string& str_val,array)
           vec.push_back(from_str<URI>(str_val));
       store[name]=vec;
     }

   }
   else
     throw ParsingFailed(FromHere(), "The type ["+type+"] of passed argument [" + arg + "] is invalid.\n"+
                         "Format should be:\n"
                         " -  for simple types:  variable_name:type=value\n"
                         " -  for array types:   variable_name:array[type]=value1,value2\n"
                         "  with possible type: [bool,unsigned,integer,real,string,uri]");
 }
 else
   throw ParsingFailed(FromHere(), "Could not parse [" + arg + "].\n"+
                       "Format should be:\n"
                       " -  for simple types:  variable_name:type=value\n"
                       " -  for array types:   variable_name:array[type]=value1,value2\n"
                       "  with possible type: [bool,unsigned,integer,real,string,uri]");
}

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

