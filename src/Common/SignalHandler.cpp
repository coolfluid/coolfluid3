// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>

#include "Common/SignalHandler.hpp"
#include "Common/XML/Protocol.hpp"
#include "Common/Foreach.hpp"
#include "Common/StringConversion.hpp"
#include "Common/BasicExceptions.hpp"

///@todo remove
#include "Common/Log.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  using namespace XML;
  
////////////////////////////////////////////////////////////////////////////////

SignalError::SignalError ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "SignalError")
{}

////////////////////////////////////////////////////////////////////////////////

std::vector < Signal > SignalHandler::list_signals () const
{
  std::vector < Signal > result;
  for ( sigmap_t::const_iterator itr = m_signals.begin() ; itr != m_signals.end() ; ++itr )
    result.push_back ( itr->second ); // add a copy of the signal to the vector
  return result;
}

////////////////////////////////////////////////////////////////////////////////

const SignalHandler::sigmap_t& SignalHandler::signals_map () const
{
  return m_signals;
}

////////////////////////////////////////////////////////////////////////////////

Signal::return_t SignalHandler::call_signal ( const Signal::id_t& sname, Signal::arg_t& sinput )
{
  return ( *signal ( sname ).signal_ptr ) ( sinput );
}

////////////////////////////////////////////////////////////////////////////////

Signal::return_t SignalHandler::call_signal ( const Signal::id_t& sname, std::vector<std::string>& sinput )
{
  sigmap_t::iterator itr = m_signals.find(sname);
  if ( itr == m_signals.end() )
    throw SignalError ( FromHere(), "Signal with name \'" + sname + "\' does not exist" );
  
  SignalFrame frame("", "", "");
  SignalFrame& options = frame.map( Protocol::Tags::key_options() );

  // extract:   variable_name:type=value   or   variable_name:array[type]=value1,value2
  boost::regex expression(  "([[:word:]]+)(\\:([[:word:]]+)(\\[([[:word:]]+)\\])?=(.*))?"  );
  boost::match_results<std::string::const_iterator> what;

  boost_foreach (const std::string& arg, sinput)
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
      CFinfo << name << ":" << type << (subtype.empty() ? std::string() : std::string("<"+subtype+">"))  << "=" << value << CFendl;

      if      (type == "bool")
        options.set_option<bool>(name,from_str<bool>(value));
      else if (type == "unsigned")
        options.set_option<Uint>(name,from_str<Uint>(value));
      else if (type == "integer")
        options.set_option<int>(name,from_str<int>(value));
      else if (type == "real")
        options.set_option<Real>(name,from_str<Real>(value));
      else if (type == "string")
        options.set_option<std::string>(name,value);
      else if (type == "uri")
        options.set_option<URI>(name,from_str<URI>(value));
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
        {
          std::vector<bool> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<bool>(str_val));
          options.set_array(name, vec, " ; ");
        }
        else if (subtype == "unsigned")
        {
          std::vector<Uint> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<Uint>(str_val));
          options.set_array(name, vec, " ; ");
        }
        else if (subtype == "integer")
        {
          std::vector<int> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<int>(str_val));
          options.set_array(name, vec, " ; ");
        }
        else if (subtype == "real")
        {
          std::vector<Real> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<Real>(str_val));
          options.set_array(name, vec, " ; ");
        }
        else if (subtype == "string")
        {
          options.set_array(name, array, " ; ");
        }
        else if (subtype == "uri")
        {
          std::vector<URI> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<URI>(str_val));
          options.set_array(name, vec, " ; ");
        }
      }
      else
        throw ParsingFailed(FromHere(), "The type ["+type+"] of passed argument [" + arg + "] for signal ["+ sname +"] is invalid.\n"+
          "Format should be:\n"
          " -  for simple types:  variable_name:type=value\n"
          " -  for array types:   variable_name:array[type]=value1,value2\n"
          "  with possible type: [bool,unsigned,integer,real,string,uri]");
    }
    else
      throw ParsingFailed(FromHere(), "Could not parse [" + arg + "] in signal ["+ sname +"].\n"+
         "Format should be:\n"
         " -  for simple types:  variable_name:type=value\n"
         " -  for array types:   variable_name:array[type]=value1,value2\n"
         "  with possible type: [bool,unsigned,integer,real,string,uri]");
  }

  call_signal(sname,frame);
  
}

////////////////////////////////////////////////////////////////////////////////

Signal& SignalHandler::signal ( const Signal::id_t& sname )
{
  sigmap_t::iterator itr = m_signals.find(sname);
  if ( itr != m_signals.end() )
    return itr->second;
  else
    throw SignalError ( FromHere(), "Signal with name \'" + sname + "\' does not exist" );
}

////////////////////////////////////////////////////////////////////////////////

const Signal& SignalHandler::signal ( const Signal::id_t& sname ) const
{
  sigmap_t::const_iterator itr = m_signals.find(sname);
  if ( itr != m_signals.end() )
    return itr->second;
  else
    throw SignalError ( FromHere(), "Signal with name \'" + sname + "\' does not exist" );
}

////////////////////////////////////////////////////////////////////////////////

bool SignalHandler::check_signal ( const Signal::id_t& sname )
{
  return ( m_signals.find(sname) != m_signals.end() );
}

////////////////////////////////////////////////////////////////////////////////

Signal::Ptr SignalHandler::regist_signal ( const Signal::id_t& sname,  const Signal::desc_t& desc, const Signal::readable_t& readable_name )
{
  // check sname complies with standard
  cf_assert( boost::algorithm::all(sname,
    boost::algorithm::is_alnum() || boost::algorithm::is_any_of("-_")) );
  
  std::string rname = readable_name;
  if (rname.empty())
    rname=sname;

  sigmap_t::iterator itr = m_signals.find (sname);

  if ( itr == m_signals.end() )
  {
    Signal& sig = m_signals[sname];

    sig.signal_ptr = Signal::Ptr( new Signal::type() );
    sig.signature = Signal::Ptr( new Signal::type() );
    sig.description = desc;
    sig.readable_name = rname;
    sig.is_read_only = false;
    sig.is_hidden = false;

    return sig.signal_ptr;
  }
  else
    return itr->second.signal_ptr;
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
