// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cstring>

#include "rapidxml/rapidxml.hpp"

#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include "Common/Assertions.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/Foreach.hpp"
#include "Common/StringConversion.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalFrame.hpp"

// makes explicit instantiation for all template functions with a same type
#define TEMPLATE_EXPLICIT_INSTANTIATION(T) \
Common_TEMPLATE template XmlNode SignalFrame::set_option<T>(const std::string&, const T&, const std::string&);\
Common_TEMPLATE template XmlNode SignalFrame::set_array<T>(const std::string&, const std::vector<T>&, const std::string&, const std::string&);\
Common_TEMPLATE template T SignalFrame::get_option<T>(const std::string&) const;\
Common_TEMPLATE template std::vector<T> SignalFrame::get_array<T>(const std::string&) const;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace XML {

////////////////////////////////////////////////////////////////////////////

SignalFrame::SignalFrame ( XmlNode xml ) :
    node(xml)
{

  if( node.is_valid() )
  {
    main_map.content = node.content->first_node( Protocol::Tags::node_map() );

    if( !main_map.content.is_valid() ) // if the main map does not exist, we create it
      main_map = node.add_node( Protocol::Tags::node_map() );
    else
    {
      const char * tag_value = Protocol::Tags::node_value();
      rapidxml::xml_node<>* value = main_map.content.content->first_node( tag_value );
      rapidxml::xml_attribute<>* attr;
      rapidxml::xml_node<>* map;

      // iterate through the children to find sub-maps
      for( ; value != nullptr ; value = value->next_sibling( tag_value ) )
      {
        attr = value->first_attribute( Protocol::Tags::attr_key() );
        map = value->first_node( );

        // if the key attribute exists and its value is not empty
        if( attr != nullptr && attr->value()[0] != '\0' &&
            map != nullptr && std::strcmp(map->name(), Protocol::Tags::node_map()) == 0 )
        {
          m_maps.insert( std::pair<std::string, SignalFrame>(attr->value(), SignalFrame(value)) );
        }
      }
    }
  }

}

////////////////////////////////////////////////////////////////////////////

SignalFrame::SignalFrame ( const std::string& target,
                           const URI& sender,
                           const URI& receiver )
{
  xml_doc = Protocol::create_doc();
  XmlNode doc_node = Protocol::goto_doc_node(*xml_doc.get());

  node = Protocol::add_signal_frame(doc_node, target, sender, receiver, false);

  main_map = node.add_node( Protocol::Tags::node_map() );
}

////////////////////////////////////////////////////////////////////////////

SignalFrame::~SignalFrame()
{

}

////////////////////////////////////////////////////////////////////////////

template<typename TYPE>
XmlNode SignalFrame::set_option ( const std::string & name, const TYPE & value,
                                  const std::string & descr )
{
  cf_assert ( node.is_valid() );

  return main_map.set_value( name, value, descr);
}

////////////////////////////////////////////////////////////////////////////

template<typename TYPE>
XmlNode SignalFrame::set_array ( const std::string & name,
                                 const std::vector<TYPE> & value,
                                 const std::string & delimiter,
                                 const std::string & descr )
{
  cf_assert ( node.is_valid() );

  return main_map.set_array(name, value, delimiter, descr);
}

////////////////////////////////////////////////////////////////////////////

template<typename TYPE>
TYPE SignalFrame::get_option ( const std::string & name ) const
{
  cf_assert ( node.is_valid() );

  return main_map.get_value<TYPE>(name);
}

////////////////////////////////////////////////////////////////////////////

template<typename TYPE>
std::vector<TYPE> SignalFrame::get_array ( const std::string & name ) const
{
  cf_assert ( node.is_valid() );

  return main_map.get_array<TYPE>(name);
}

////////////////////////////////////////////////////////////////////////////

bool SignalFrame::has_map ( const std::string & name ) const
{
  return m_maps.find(name) != m_maps.end();
}

////////////////////////////////////////////////////////////////////////////

SignalFrame & SignalFrame::map ( const std::string & name )
{
  cf_assert ( node.is_valid() );
  cf_assert ( !name.empty() );

  if( m_maps.find(name) == m_maps.end() ) // if the node does not exist yet, we create it
  {
    XmlNode node = main_map.content.add_node( Protocol::Tags::node_value() );
    node.set_attribute( Protocol::Tags::attr_key(), name );
    m_maps[name] = SignalFrame(node); // SignalFrame() adds a map under the node
  }

  return m_maps[name];
}

////////////////////////////////////////////////////////////////////////////

const SignalFrame & SignalFrame::map ( const std::string & name ) const
{
  cf_assert ( node.is_valid() );
  cf_assert ( !name.empty() );

  std::map<std::string, SignalFrame>::const_iterator it_map = m_maps.find(name);
  cf_assert ( it_map != m_maps.end() );
  return it_map->second;
}

////////////////////////////////////////////////////////////////////////////

SignalFrame SignalFrame::create_reply ( const URI & sender )
{
  cf_assert ( node.is_valid() );

  URI sender_uri(sender);

  // if the sender parameter is not valid, we use the one of the signal
  if( sender_uri.scheme() != URI::Scheme::CPATH || sender_uri.path().empty())
  {
    rapidxml::xml_attribute<>* attr = node.content->first_attribute("receiver");

    if(attr == nullptr)
      throw XmlError(FromHere(), "Could not found a sender");

    sender_uri = URI(attr->value());
  }

  SignalFrame reply(Protocol::add_reply_frame( node ));

  reply.node.set_attribute("sender", sender_uri.string() );

  return reply;
}

////////////////////////////////////////////////////////////////////////////

SignalFrame SignalFrame::get_reply () const
{
  cf_assert( node.is_valid() );

  const char * frame_tag = Protocol::Tags::node_frame();

  XmlNode reply( node.content->next_sibling(frame_tag) );

  for( ; reply.is_valid() ; reply.content = reply.content->next_sibling(frame_tag) )
  {
    rapidxml::xml_attribute<>* attr = reply.content->first_attribute( "type" );

    if( attr != nullptr && std::strcmp(attr->value(), Protocol::Tags::node_type_reply()) == 0 )
      return SignalFrame(reply);
  }

  return SignalFrame();
}

////////////////////////////////////////////////////////////////////////////

void SignalFrame::insert( std::vector<std::string>& input )
{
  // extract:   variable_name:type=value   or   variable_name:array[type]=value1,value2
  boost::regex expression(  "([[:word:]]+)(\\:([[:word:]]+)(\\[([[:word:]]+)\\])?=(.*))?"  );
  boost::match_results<std::string::const_iterator> what;

  boost_foreach (const std::string& arg, input)
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
        set_option<bool>(name,from_str<bool>(value));
      else if (type == "unsigned")
        set_option<Uint>(name,from_str<Uint>(value));
      else if (type == "integer")
        set_option<int>(name,from_str<int>(value));
      else if (type == "real")
        set_option<Real>(name,from_str<Real>(value));
      else if (type == "string")
        set_option<std::string>(name,value);
      else if (type == "uri")
        set_option<URI>(name,from_str<URI>(value));
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
          set_array(name, vec, " ; ");
        }
        else if (subtype == "unsigned")
        {
          std::vector<Uint> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<Uint>(str_val));
          set_array(name, vec, " ; ");
        }
        else if (subtype == "integer")
        {
          std::vector<int> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<int>(str_val));
          set_array(name, vec, " ; ");
        }
        else if (subtype == "real")
        {
          std::vector<Real> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<Real>(str_val));
          set_array(name, vec, " ; ");
        }
        else if (subtype == "string")
        {
          set_array(name, array, " ; ");
        }
        else if (subtype == "uri")
        {
          std::vector<URI> vec; vec.reserve(array.size());
          boost_foreach(const std::string& str_val,array)
            vec.push_back(from_str<URI>(str_val));
          set_array(name, vec, " ; ");
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
}

////////////////////////////////////////////////////////////////////////////

/// explicit instantiation to avoid missing symbols on certain compilers
TEMPLATE_EXPLICIT_INSTANTIATION( bool );
TEMPLATE_EXPLICIT_INSTANTIATION( int );
TEMPLATE_EXPLICIT_INSTANTIATION( std::string );
TEMPLATE_EXPLICIT_INSTANTIATION( CF::Uint );
TEMPLATE_EXPLICIT_INSTANTIATION( CF::Real );
TEMPLATE_EXPLICIT_INSTANTIATION( CF::Common::URI );

/////////////////////////////////////////////////////////////////////////////////

} // XML
} // Common
} // CF

#undef TEMPLATE_EXPLICIT_INSTANTIATION
