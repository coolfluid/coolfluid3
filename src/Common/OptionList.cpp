// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

#include "Common/Foreach.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/XML/Protocol.hpp"
#include "Common/OptionArray.hpp"

#include "Common/Component.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionT.hpp"
#include "Common/StringConversion.hpp"

#include "Common/OptionList.hpp"

namespace CF {
namespace Common {

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
  Option::Ptr opt;
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
  Option::ConstPtr opt;
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

void OptionList::configure_option(const std::string& pname, const boost::any& val)
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
  Option::Ptr prop = itr->second;

  // update the value and trigger its actions
  prop->change_value(val);
}

////////////////////////////////////////////////////////////////////////////////

std::string OptionList::list_options()
{
  std::string opt_list="";
  Uint cnt(0);
  foreach_container( (const std::string& name) (Option::Ptr option) , *this )
  {
    if (cnt > 0)
      opt_list=opt_list+"\n";

    if (option->tag() == XML::Protocol::Tags::node_array())
    {
      OptionArray::Ptr array_option = boost::dynamic_pointer_cast<OptionArray>(option);
      std::string values=array_option->value_str();
      boost::algorithm::replace_all(values, array_option->separator(), ",");
      opt_list = opt_list + name + ":array[" + array_option->elem_type() + "]=" + values;
    }
    else
    {
      opt_list = opt_list + name + ":" + option->type() + "=" + option->value_str();
    }
//
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
    options.add_option< OPTION_TYPE >(name, value);
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

  set_option_to_list< OptionArrayT<TYPE> >( name, vec, options);
}

//////////////////////////////////////////////////////////////////////////////

void OptionList::fill_from_vector( const std::vector<std::string> & args )
{
  // extract:   variable_name:type=value   or   variable_name:array[type]=value1,value2
  boost::regex expression(  "([[:word:]]+)(\\:([[:word:]]+)(\\[([[:word:]]+)\\])?=(.*))?"  );
  boost::match_results<std::string::const_iterator> what;

  boost_foreach (const std::string& arg, args)
  {

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
        typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
        boost::char_separator<char> sep(",");
        Tokenizer tokens(value, sep);

        for (Tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
        {
          array.push_back(*tok_iter);
          boost::algorithm::trim(array.back()); // remove leading and trailing spaces
        }

        if (subtype == "bool")
          set_array_to_list<bool>(name, array, *this);
        else if (subtype == "unsigned")
          set_array_to_list<Uint>(name, array, *this);
        else if (subtype == "integer")
          set_array_to_list<int>(name, array, *this);
        else if (subtype == "real")
          set_array_to_list<Real>(name, array, *this);
        else if (subtype == "string")
          set_option_to_list< OptionArrayT<std::string> >(name, array, *this);
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
}


//// "Magic" add_option implementation

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
//  typedef OptionArrayT<T> type;
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
//  return dynamic_cast<OptionType>(add_option<OptionType>(name, default_value));
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

} // Common
} // CF

