// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>
#include <boost/any.hpp>

#include "common/Foreach.hpp"
#include "common/BasicExceptions.hpp"
#include "common/XML/Protocol.hpp"
#include "common/OptionArray.hpp"

#include "common/Option.hpp"
#include "common/Component.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionURI.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionT.hpp"
#include "common/StringConversion.hpp"

#include "common/OptionList.hpp"

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////////////

const Option & OptionList::option( const std::string& pname) const
{
  OptionStorage_t::const_iterator itr = store.find(pname);

  if ( itr != store.end() )
    return *itr->second.get();
  else
  {
    std::string msg;
    msg += "Option with name ["+pname+"] not found. Available options are:\n";
    OptionStorage_t::const_iterator it = store.begin();
    for (; it!=store.end(); it++)
      msg += "  - " + it->first + "\n";
    throw ValueNotFound(FromHere(),msg);
  }

}

////////////////////////////////////////////////////////////////////////////////

Option & OptionList::option( const std::string& pname)
{
  OptionStorage_t::iterator itr = store.find(pname);

  if ( itr != store.end() )
    return *itr->second.get();
  else
  {
    std::string msg;
    msg += "Option with name ["+pname+"] not found. Available options are:\n";
    OptionStorage_t::iterator it = store.begin();
    for (; it!=store.end(); it++)
      msg += "  - " + it->first + "\n";
    throw ValueNotFound(FromHere(),msg);
  }

}

////////////////////////////////////////////////////////////////////////////////

void OptionList::erase( const std::string& name)
{
  OptionStorage_t::iterator itr = store.find(name);
  if ( itr != store.end() )
    store.erase(itr);
  else
    throw ValueNotFound(FromHere(), "Option with name [" + name + "] not found" );
}

////////////////////////////////////////////////////////////////////////////////

Option & OptionList::operator [] (const std::string & pname)
{
  boost::shared_ptr<Option> opt;
  OptionStorage_t::iterator itr = store.find(pname);

  if ( itr != store.end() )
    opt = itr->second;
  else
  {
    std::string msg;
    msg += "Option with name ["+pname+"] not found. Available options are:\n";
    OptionStorage_t::const_iterator it = store.begin();
    for (; it!=store.end(); it++)
      msg += "  - " + it->first + "\n";
    throw ValueNotFound(FromHere(),msg);
  }

  return *opt.get();
}

////////////////////////////////////////////////////////////////////////////////

const Option & OptionList::operator [] (const std::string & pname) const
{
  boost::shared_ptr<Option const> opt;
  OptionStorage_t::const_iterator itr = store.find(pname);

  if ( itr != store.end() )
    opt = itr->second;
  else
  {
    std::string msg;
    msg += "Option with name ["+pname+"] not found. Available options are:\n";
    OptionStorage_t::const_iterator it = store.begin();
    for (; it!=store.end(); it++)
      msg += "  - " + it->first + "\n";
    throw ValueNotFound(FromHere(),msg);
  }
  return *opt.get();
}

////////////////////////////////////////////////////////////////////////////////

void OptionList::set(const std::string& pname, const boost::any& val)
{
  OptionStorage_t::iterator itr = store.find(pname);
  if (itr == store.end())
  {
    std::string msg;
    msg += "Option with name ["+pname+"] not found. Available options are:\n";
    if (store.size())
    {
      OptionStorage_t::iterator it = store.begin();
      for (; it!=store.end(); it++)
        msg += "  - " + it->first + "\n";
    }
    throw ValueNotFound(FromHere(),msg);
  }
  boost::shared_ptr<Option> prop = itr->second;

  // update the value and trigger its actions
  prop->change_value(val);
}

////////////////////////////////////////////////////////////////////////////////

std::string OptionList::list_options() const
{
  std::string opt_list="";
  Uint cnt(0);
  foreach_container( (const std::string& name) (const boost::shared_ptr<Option> option) , *this )
  {
    if (cnt > 0)
    {
      opt_list=opt_list+"\n";
    }

    opt_list = "  - " + opt_list + name + ":" + option->type() + "=" + option->value_str();

    if (option->has_restricted_list())
    {
      std::string rest_list="";
      std::vector<boost::any>::iterator ir = option->restricted_list().begin();
      for( ; ir != option->restricted_list().end() ; ++ir )
        rest_list= rest_list + " " + boost::any_cast<std::string>(*ir);
      opt_list = opt_list + " restrictions:" + rest_list;
    }

    ++cnt;
  }
  return opt_list;
}

//////////////////////////////////////////////////////////////////////////////

template< typename OPTION_TYPE >
void set_option_to_list( const std::string & name,
                         const typename OPTION_TYPE::value_type & value,
                         OptionList & options )
{
  if( !options.check(name) )
    options.add(name, value);
  else
    options[name].change_value( value );
}

//////////////////////////////////////////////////////////////////////////////

template< typename TYPE >
void set_array_to_list( const std::string & name,
                        const std::vector<std::string> & values,
                        OptionList & options )
{
  std::vector<TYPE> vec;
  vec.reserve(values.size());

  boost_foreach( const std::string& str_val, values )
    vec.push_back( from_str<TYPE>(str_val) );

  set_option_to_list< OptionArray<TYPE> >( name, vec, options);
}

//////////////////////////////////////////////////////////////////////////////

void OptionList::set( const std::string & arg )
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

    //CFinfo << name << ":" << type << (subtype.empty() ? std::string() : std::string("["+subtype+"]"))  << "=" << value << CFendl;

    if      (type == "bool")
      set_option_to_list< OptionT<bool> >(name, from_str<bool>( value ), *this );
    else if (type == "unsigned")
      set_option_to_list< OptionT<Uint> >(name, from_str<Uint>( value ), *this );
    else if (type == "integer")
      set_option_to_list< OptionT<int> >(name, from_str<int>( value ), *this );
    else if (type == "real")
      set_option_to_list< OptionT<Real> >(name, from_str<Real>( value ), *this );
    else if (type == "string")
      set_option_to_list< OptionT<std::string> >(name, value, *this );
    else if (type == "uri")
      set_option_to_list< OptionURI >(name, from_str<URI>( value ), *this );
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
        set_array_to_list<bool>(name, array, *this);
      else if (subtype == "unsigned")
        set_array_to_list<Uint>(name, array, *this);
      else if (subtype == "integer")
        set_array_to_list<int>(name, array, *this);
      else if (subtype == "real")
        set_array_to_list<Real>(name, array, *this);
      else if (subtype == "string")
        set_option_to_list< OptionArray<std::string> >(name, array, *this);
      else if (subtype == "uri")
        set_array_to_list<URI>(name, array, *this);

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

void OptionList::set( const std::vector<std::string> & args )
{
  boost_foreach (const std::string& arg, args)
  {
    set(arg);
  }
}

////////////////////////////////////////////////////////////////////////////////

//// "Magic" add implementation

//// Specialization for URI
//template<>
//struct OptionList::SelectOptionType<URI>
//{
//  typedef OptionURI type;
//};

//// Specialization for OptionArray
//template<typename T>
//struct OptionList::SelectOptionType< std::vector<T> >
//{
//  typedef OptionArray<T> type;
//};

///// Shortcut to choose the appropriate value type
//template<typename T>
//struct SelectValueType
//{
//  typedef typename OptionList::SelectOptionType<T>::type::value_type type;
//};

//template<typename T>
//typename OptionList::SelectOptionType<T>::type::Ptr OptionList::add_opt(
//  const std::string& name, const typename SelectOptionType<T>::type::value_type & default_value)
//{
//  typedef typename OptionList::SelectOptionType<T>::type OptionType;
//  return dynamic_cast<OptionType>(add(name, default_value));
//}

//#define EXPLICIT_TEMPLATE_INSTANCIATION(T) \
//  Common_TEMPLATE template OptionList::SelectOptionType<T>::type::Ptr OptionList::add_opt<T>(const std::string&, const typename OptionList::SelectOptionType<T>::type&);\
//Common_TEMPLATE template OptionList::SelectOptionType<std::vector<T> >::type::Ptr OptionList::add_opt<std::vector<T> >(const std::string&, const typename OptionList::SelectOptionType<std::vector<T> >::type&)

//Common_TEMPLATE template class OptionList::SelectOptionType< std::vector<bool> >;
//Common_TEMPLATE template class OptionList::SelectOptionType< std::vector<int> >;
//Common_TEMPLATE template class OptionList::SelectOptionType< std::vector<Uint> >;
//Common_TEMPLATE template class OptionList::SelectOptionType< std::vector<Real> >;
//Common_TEMPLATE template class OptionList::SelectOptionType< std::vector<std::string> >;
//Common_TEMPLATE template class OptionList::SelectOptionType< std::vector<URI> >;

//EXPLICIT_TEMPLATE_INSTANCIATION(bool);
//EXPLICIT_TEMPLATE_INSTANCIATION(int);
//EXPLICIT_TEMPLATE_INSTANCIATION(Uint);
//EXPLICIT_TEMPLATE_INSTANCIATION(Real);
//EXPLICIT_TEMPLATE_INSTANCIATION(std::string);
//EXPLICIT_TEMPLATE_INSTANCIATION(URI);

//#undef EXPLICIT_TEMPLATE_INSTANCIATION

/////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

