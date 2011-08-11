// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

#include "rapidxml/rapidxml.hpp"

#include "Common/Assertions.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/Foreach.hpp"
#include "Common/Option.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/StringConversion.hpp"

#include "Common/XML/CastingFunctions.hpp"
#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalFrame.hpp"

#include "Common/XML/SignalOptions.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace XML {

//////////////////////////////////////////////////////////////////////////////

/// Creates an @c #OptionT option with a value of type TYPE.
/// @param name Option name
/// @param pretty_name The option pretty name
/// @param descr Option description
/// @param node The value node. If it has a sibling node, this node is taken as
/// the restricted values list.
/// @return Returns the created option.
/// @author Quentin Gasper.
template<typename TYPE>
Option::Ptr make_option_t(const std::string & name, const std::string & pretty_name,
                          const std::string & descr, XmlNode & node)
{
  TYPE value;
  to_value(node, value);
  XmlNode restr_node = Map(node.content->parent())
      .find_value(Protocol::Tags::key_restricted_values(), Protocol::Tags::node_array());


  Option::Ptr option(new Common::OptionT<TYPE>(name, value));

  option->description( descr );
  option->pretty_name( pretty_name );

  if(restr_node.is_valid())
  {
    std::string delimiter;
    std::vector<TYPE> restr_list = Map().array_to_vector<TYPE>( restr_node, &delimiter );

    typename std::vector<TYPE>::iterator it;

    option->separator(delimiter);

//    option->restricted_list().push_back( value );

    for( it = restr_list.begin() ; it != restr_list.end() ; ++it)
      option->restricted_list().push_back( *it );
  }

  return option;
}

////////////////////////////////////////////////////////////////////////////

/// Creates an @c #OptionArrayT option with values of type TYPE.
/// @param name Option name
/// @param pretty_name The option pretty name
/// @param descr Option description
/// @param node The value node. If it has a sibling node, this node is taken as
/// the restricted values list.
/// @return Returns the created option.
/// @author Quentin Gasper.
template<typename TYPE>
typename OptionArrayT<TYPE>::Ptr make_option_array_t(const std::string & name,
                                                     const std::string & pretty_name,
                                                     const std::string & descr,
                                                     const XmlNode & node)
{
  std::string delimiter;
  std::vector<TYPE> value = Map().array_to_vector<TYPE>(node, &delimiter);

  typename OptionArrayT<TYPE>::Ptr option(new OptionArrayT<TYPE>(name, value));

  option->description( descr );
  option->pretty_name( pretty_name );
  option->separator(delimiter);

  XmlNode restr_node = Map(node.content->parent()).find_value(Protocol::Tags::key_restricted_values(), Protocol::Tags::node_array());


  if(restr_node.is_valid())
  {
    std::vector<TYPE> restr_list = Map().array_to_vector<TYPE>( restr_node );

    typename std::vector<TYPE>::iterator it;

    option->restricted_list().push_back( value );

    for( it = restr_list.begin() ; it != restr_list.end() ; ++it)
      option->restricted_list().push_back( *it );
  }


  return option;
}

//////////////////////////////////////////////////////////////////////////////

Option::Ptr SignalOptions::xml_to_option( const XmlNode & node )
{
  cf_assert( node.is_valid() );

  bool advanced; // indicates whether the option should be advanced or not
  Option::Ptr option;
  std::string opt_type( Map::get_value_type(node) ); // option type

  // get some attributes
  rapidxml::xml_attribute<>* key_attr = node.content->first_attribute( Protocol::Tags::attr_key() );
  rapidxml::xml_attribute<>* descr_attr = node.content->first_attribute( Protocol::Tags::attr_descr() );
  rapidxml::xml_attribute<>* p_name_attr = node.content->first_attribute( Protocol::Tags::attr_pretty_name() );

  rapidxml::xml_attribute<>* mode_attr = node.content->first_attribute( "mode" );

  // check the key is present and not empty
  if( is_null(key_attr) || key_attr->value()[0] == '\0' )
    throw ProtocolError(FromHere(), "The option key is missing or empty.");

  std::string key_str( key_attr->value() ); // option name

  // get the description and the pretty name (if present)
  std::string descr_str( is_not_null(descr_attr) ? descr_attr->value() : "" );
  std::string pretty_name( is_not_null(p_name_attr) ? p_name_attr->value() : "" );

  advanced = is_null(mode_attr) || std::strcmp(mode_attr->value(), "adv") == 0;

  // if it is a single value
  if( Map::is_single_value(node) )
  {
    XmlNode type_node( node.content->first_node(opt_type.c_str()) );

    if( opt_type == Protocol::Tags::type<bool>() )
      option = make_option_t<bool>(key_str, pretty_name, descr_str, type_node);
    else if( opt_type == Protocol::Tags::type<int>() )
      option = make_option_t<int>(key_str, pretty_name, descr_str, type_node);
    else if( opt_type == Protocol::Tags::type<Uint>() )
      option = make_option_t<Uint>(key_str, pretty_name, descr_str, type_node);
    else if( opt_type == Protocol::Tags::type<Real>() )
      option = make_option_t<Real>(key_str, pretty_name, descr_str, type_node);
    else if( opt_type == Protocol::Tags::type<std::string>() )
      option = make_option_t<std::string>(key_str, pretty_name, descr_str, type_node);
    else if( opt_type == Protocol::Tags::type<URI>() )
    {
      // in the case of a URI value, we have to create an OptionURI and
      // check for supported schemes

      URI value;
      rapidxml::xml_attribute<>* schemes_attr = nullptr;
      std::vector<std::string> schemes;
      std::vector<std::string>::iterator it;
      OptionURI::Ptr option_uri;

      XmlNode restr_node = Map(node).find_value(Protocol::Tags::key_restricted_values(), Protocol::Tags::node_array());

      std::vector<URI> restr_list;

      // cast the value
      try
      {
        to_value(type_node, value);
      }
      catch( boost::bad_lexical_cast & e)
      {
        std::string value( type_node.content->value() );
        throw CastingFailed( FromHere(), "Unable to cast [" + value + "] to type [URI].");
      }

      // create the option
      option_uri = OptionURI::Ptr (new Common::OptionURI(key_str, value));

      option_uri->description( descr_str );
      option_uri->pretty_name( pretty_name );

      // add the list of restricted values (if present)
      if( restr_node.is_valid() )
      {
        restr_list = Map().array_to_vector<URI>( restr_node );

        std::vector<URI>::iterator it;

        option->restricted_list().push_back( value );

        for( it = restr_list.begin() ; it != restr_list.end() ; ++it)
          option->restricted_list().push_back( *it );
      }

      schemes_attr = node.content->first_attribute( Protocol::Tags::attr_uri_schemes() );

      // add the supported schems (if present)
      if( is_not_null(schemes_attr) && schemes_attr->value_size() != 0 )
      {
        std::string schemes_str(schemes_attr->value());

        boost::algorithm::split(schemes, schemes_str, boost::algorithm::is_any_of(","));

        for(it = schemes.begin() ; it != schemes.end() ; it++)
        {
          URI::Scheme::Type scheme = URI::Scheme::Convert::instance().to_enum(*it);

          if( scheme == URI::Scheme::INVALID )
            throw CastingFailed(FromHere(), "[" + *it + "] is not a valid scheme.");

          option_uri->supported_protocol(scheme);
        }
      }

      option = option_uri;
    }
    else
      throw ShouldNotBeHere(FromHere(), opt_type + ": Unknown option type");

  }
  else if( Map::is_array_value(node) ) // it is an array value
  {
    if(opt_type == Protocol::Tags::type<bool>() )
      option = make_option_array_t<bool>(key_str, pretty_name, descr_str, node);
    else if(opt_type == Protocol::Tags::type<int>() )
      option = make_option_array_t<int>(key_str, pretty_name, descr_str, node);
    else if(opt_type == Protocol::Tags::type<Uint>() )
      option = make_option_array_t<Uint>(key_str, pretty_name, descr_str, node);
    else if(opt_type == Protocol::Tags::type<Real>() )
      option = make_option_array_t<Real>(key_str, pretty_name, descr_str, node);
    else if(opt_type == Protocol::Tags::type<std::string>() )
      option = make_option_array_t<std::string>(key_str, pretty_name, descr_str, node);
    else if(opt_type == Protocol::Tags::type<URI>() )
      option = make_option_array_t<URI>(key_str, pretty_name, descr_str, node);
    else
      throw ShouldNotBeHere(FromHere(), opt_type + ": Unknown option type");

  }
  else
    throw ProtocolError(FromHere(), "Node [" + std::string(node.content->name()) +"] does not "
                        "represent either a single value nor an array value.");

  // if the option is not advanced, mark it as basic
  if( !advanced && option.get() != nullptr )
    option->mark_basic();


  return option;
}

////////////////////////////////////////////////////////////////////////////////////////////

/// Adds an option to an XML map.
/// @param opt_map Map the option should be added to
/// @param opt Option to add
/// @param is_array If @c true, the option is treated as an array.
/// @author Quentin Gasper.
template<typename TYPE>
void add_opt_to_xml( Map& opt_map, Option::Ptr opt, bool is_array)
{
  cf_assert( opt_map.content.is_valid() );
  cf_assert( is_not_null( opt.get() ) );

  XmlNode value_node;
  bool basic = opt->has_tag("basic");
  std::string desc = opt->description();


  if( !is_array )
    value_node = opt_map.set_value<TYPE>( opt->name(), opt->value<TYPE>(), desc );
  else
  {
    typename OptionArrayT<TYPE>::Ptr array = boost::dynamic_pointer_cast<OptionArrayT<TYPE> >(opt);
    value_node = opt_map.set_array<TYPE>( opt->name(), array->value_vect(), opt->separator(), desc );
  }

  value_node.set_attribute( Protocol::Tags::attr_pretty_name(), opt->pretty_name() );
  value_node.set_attribute( "is_option", to_str<bool>(true) );
  value_node.set_attribute( "mode", (basic ? "basic" : "adv") );

  if( opt->has_restricted_list() )
  {
    Map value_map(value_node);
    std::vector<TYPE> vect;
    std::vector<boost::any>::iterator it = opt->restricted_list().begin();

    for( ; it != opt->restricted_list().end() ; ++it )
      vect.push_back( boost::any_cast<TYPE>(*it) );

    value_map.set_array( Protocol::Tags::key_restricted_values(), vect, opt->separator() );
  }

  if( std::strcmp( opt->tag(), Protocol::Tags::type<URI>() ) == 0)
  {
    std::vector<URI::Scheme::Type> prots = boost::dynamic_pointer_cast<OptionURI>(opt)->supported_protocols();
    std::vector<URI::Scheme::Type>::iterator it = prots.begin();

    for( ; it != prots.end() ; it++)
      value_node.set_attribute( Protocol::Tags::attr_uri_schemes(),
                                URI::Scheme::Convert::instance().to_str(*it));
   }
}

//////////////////////////////////////////////////////////////////////////////

SignalOptions::SignalOptions()
  : OptionList()
{

}

//////////////////////////////////////////////////////////////////////////////

SignalOptions::SignalOptions(const OptionList &list)
  : OptionList(list)
{

}

//////////////////////////////////////////////////////////////////////////////

SignalOptions::SignalOptions( SignalFrame & frame,  const std::string & name )
  : OptionList()
{
  cf_assert( frame.node.is_valid() );

  if( name.empty() )
    main_map = frame.map( Protocol::Tags::key_options() ).main_map;
  else
    main_map = frame.map( name ).main_map;

  rapidxml::xml_node<> * value_node = main_map.content.content->first_node();

  for( ; is_not_null(value_node) ; value_node = value_node->next_sibling() )
  {
    Option::Ptr option = xml_to_option( XmlNode(value_node) );

    if( check(option->name()) )
      throw ValueExists(FromHere(), "Option [" + option->name() + "] already exists.");

    store[ option->name() ] = option;
  }
}

//////////////////////////////////////////////////////////////////////////////

SignalOptions::~SignalOptions()
{
  flush();
}

//////////////////////////////////////////////////////////////////////////////

SignalFrame SignalOptions::create_frame( const std::string & name,
                                            const URI & sender,
                                            const URI & receiver ) const
{
  SignalFrame frame(name, sender, receiver);
  Map map = frame.map( Protocol::Tags::key_options() ).main_map;

  add_to_map( map, *this );

  return frame;
}

//////////////////////////////////////////////////////////////////////////////

void SignalOptions::add_to_map( Map & map, const OptionList & list )
{
  cf_assert( map.content.is_valid() );

  for( OptionStorage_t::const_iterator it = list.begin() ; it != list.end() ; ++it )
  {
    Option::Ptr option = it->second;
    std::string type = option->tag();
    bool is_array = ( type == Protocol::Tags::node_array() );

    if( is_array )
      type = boost::dynamic_pointer_cast<OptionArray>( option )->elem_type();

    if ( type == Protocol::Tags::type<bool>() )
      add_opt_to_xml<bool>( map, option, is_array );
    else if ( type == Protocol::Tags::type<int>() )
      add_opt_to_xml<int>( map, option, is_array );
    else if ( type == Protocol::Tags::type<Uint>() )
      add_opt_to_xml<Uint>( map, option, is_array );
    else if ( type == Protocol::Tags::type<Real>() )
      add_opt_to_xml<Real>( map, option, is_array );
    else if ( type == Protocol::Tags::type<std::string>() )
      add_opt_to_xml<std::string>( map, option, is_array );
    else if ( type == Protocol::Tags::type<URI>() )
      add_opt_to_xml<URI>( map, option, is_array );
    else
      throw ShouldNotBeHere(FromHere(), "Unable to handle options of type [" + type +"].");
  }
}

//////////////////////////////////////////////////////////////////////////////

template<typename TYPE>
TYPE SignalOptions::value( const std::string & name ) const
{
  return option(name).value<TYPE>();
}

//////////////////////////////////////////////////////////////////////////////

template<typename TYPE>
std::vector<TYPE> SignalOptions::array( const std::string & name ) const
{
  typename OptionArrayT<TYPE>::ConstPtr opt;

  opt = boost::dynamic_pointer_cast< const OptionArrayT<TYPE> >(option(name).shared_from_this());

  cf_assert( is_not_null(opt) ); // check the casting went OK

  return opt->value_vect();
}

//////////////////////////////////////////////////////////////////////////////

void SignalOptions::flush()
{
  if( main_map.content.is_valid() )
    add_to_map( main_map, *this );
}

//////////////////////////////////////////////////////////////////////////////

SignalFrame SignalOptions::create_reply_to( SignalFrame & frame, const URI & sender ) const
{
  cf_assert( main_map.content.is_valid() );

  SignalFrame reply = frame.create_reply( sender );
  Map map = reply.map( Protocol::Tags::key_options() ).main_map;

  add_to_map(map, *this);

  return reply;
}

//////////////////////////////////////////////////////////////////////////////

#define TEMPLATE_EXPLICIT_INSTANTIATION(T) \
  Common_TEMPLATE template T SignalOptions::value<T>(const std::string&) const;\
  Common_TEMPLATE template std::vector<T> SignalOptions::array<T>(const std::string&) const

/// explicit instantiation to avoid missing symbols on certain compilers
TEMPLATE_EXPLICIT_INSTANTIATION( bool );
TEMPLATE_EXPLICIT_INSTANTIATION( int );
TEMPLATE_EXPLICIT_INSTANTIATION( std::string );
TEMPLATE_EXPLICIT_INSTANTIATION( CF::Uint );
TEMPLATE_EXPLICIT_INSTANTIATION( CF::Real );
TEMPLATE_EXPLICIT_INSTANTIATION( URI );

#undef TEMPLATE_EXPLICIT_INSTANTIATION

//////////////////////////////////////////////////////////////////////////////

} // XML
} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////
