// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

#include "rapidxml/rapidxml.hpp"

#include "common/Assertions.hpp"
#include "common/BasicExceptions.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/Option.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionFactory.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/StringConversion.hpp"

#include "common/XML/CastingFunctions.hpp"
#include "common/XML/Protocol.hpp"
#include "common/XML/SignalFrame.hpp"

#include "common/XML/SignalOptions.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
namespace XML {

//////////////////////////////////////////////////////////////////////////////

/// Creates an @c #OptionT option
/// @param name Option name
/// @param node The value node.
/// @return Returns the created option.
/// @author Quentin Gasper., Bart Janssens
boost::shared_ptr<Option> make_option(const std::string & name, XmlNode & node)
{
  return OptionFactory::instance().create_option(name, node.content->name(), std::string(node.content->value()));
}

////////////////////////////////////////////////////////////////////////////

/// Creates an @c #OptionArray option
/// @param name Option name
/// @param node The value node.
/// @return Returns the created option.
/// @author Quentin Gasper, Bart Janssens
boost::shared_ptr< Option > make_option_array(const std::string & name, const XmlNode & node)
{
  std::string delimiter;
  std::vector<std::string> value = Map().array_to_vector<std::string>(node, &delimiter);

  boost::shared_ptr<Option> option = OptionFactory::instance().create_option(name, "array[" + std::string(Map::get_value_type(node)) + "]", value);

  option->separator(delimiter);

  return option;
}

//////////////////////////////////////////////////////////////////////////////

/// Set description, pretty name and restricted list, if any
/// @param option THe option for which to set the attributes
/// @param pretty_name The option pretty name
/// @param descr Option description
/// @param node The value node. If it has a sibling node, this node is taken as
/// the restricted values list.
/// @author Quentin Gasper, Bart Janssens
void set_option_attributes(Option& option,
                  const std::string & pretty_name,
                  const std::string & descr,
                  const XmlNode & node)
{
  option.description( descr );
  option.pretty_name( pretty_name );

  XmlNode restr_node = Map(node).find_value(Protocol::Tags::key_restricted_values());//Protocol::Tags::node_array()

  if(restr_node.is_valid())
  {
    std::string delimiter;
    std::vector<std::string> restr_list = Map().array_to_vector<std::string>( restr_node, &delimiter );

    option.separator(delimiter);
    option.set_restricted_list_str(restr_list);
  }
}

//////////////////////////////////////////////////////////////////////////////

boost::shared_ptr<Option> SignalOptions::xml_to_option( const XmlNode & node )
{
  cf3_assert( node.is_valid() );

  bool advanced; // indicates whether the option should be advanced or not
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

  boost::shared_ptr<Option> option;

  // if it is a single value
  if( Map::is_single_value(node) )
  {
    XmlNode type_node( node.content->first_node(opt_type.c_str()) );
    option = make_option(key_str, type_node);
    boost::shared_ptr<OptionURI> option_uri = boost::dynamic_pointer_cast<OptionURI>(option);

    // Add the schemes if we have a URI
    if( is_not_null(option_uri) )
    {
      // in the case of a URI value, we have to create an OptionURI and
      // check for supported schemes

      rapidxml::xml_attribute<>* schemes_attr = nullptr;
      std::vector<std::string> schemes;
      std::vector<std::string>::iterator it;

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
    }
  }
  else if( Map::is_array_value(node) ) // it is an array value
  {
    option = make_option_array(key_str, node);
  }
  else
    throw ProtocolError(FromHere(), "Node [" + std::string(node.content->name()) +"] does not "
                        "represent either a single value nor an array value.");

  // if the option is not advanced, mark it as basic
  if( !advanced && option.get() != nullptr )
    option->mark_basic();

  if(is_not_null(option))
    set_option_attributes(*option, pretty_name, descr_str, node);

  return option;
}

////////////////////////////////////////////////////////////////////////////////////////////

/// Adds an option to an XML map.
/// @param opt_map Map the option should be added to
/// @param opt Option to add
/// @param is_array If @c true, the option is treated as an array.
/// @author Quentin Gasper.
void add_opt_to_xml( Map& opt_map, boost::shared_ptr<Option> opt, bool is_array)
{
  cf3_assert( opt_map.content.is_valid() );
  cf3_assert( is_not_null( opt.get() ) );

  XmlNode value_node;
  bool basic = opt->has_tag("basic");
  std::string desc = opt->description();


  if( !is_array )
  {
    value_node = opt_map.set_value( opt->name(), opt->type(), opt->value_str(), desc );
  }
  else
  {
    value_node = opt_map.set_array( opt->name(), opt->element_type(), opt->value_str(), opt->separator(), desc );
  }

  value_node.set_attribute( Protocol::Tags::attr_pretty_name(), opt->pretty_name() );
  value_node.set_attribute( "is_option", "true" );
  value_node.set_attribute( "mode", (basic ? "basic" : "adv") );

  if( opt->has_restricted_list() )
  {
    Map value_map(value_node);
    value_map.set_array( Protocol::Tags::key_restricted_values(), opt->element_type(), opt->restricted_list_str(), opt->separator() );
  }

  if(  opt->type() == common::class_name<URI>() )
  {
    boost::shared_ptr<OptionURI> uri_opt = boost::dynamic_pointer_cast<OptionURI>(opt);
    if(is_null(uri_opt))
      throw ShouldNotBeHere(FromHere(), "Option " + opt->name() + " has type " + opt->type() + " but can't be cast to OptionURI. TypeID is " + typeid(opt.get()).name());
    std::vector<URI::Scheme::Type> prots = uri_opt->supported_protocols();
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
  cf3_assert( frame.node.is_valid() );

  if( name.empty() )
    main_map = frame.map( Protocol::Tags::key_options() ).main_map;
  else
    main_map = frame.map( name ).main_map;

  rapidxml::xml_node<> * value_node = main_map.content.content->first_node();

  for( ; is_not_null(value_node) ; value_node = value_node->next_sibling() )
  {
    boost::shared_ptr<Option> option = xml_to_option( XmlNode(value_node) );

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
  cf3_assert( map.content.is_valid() );

  for( OptionStorage_t::const_iterator it = list.begin() ; it != list.end() ; ++it )
  {
    boost::shared_ptr<Option> option = it->second;

    bool is_array = boost::starts_with( option->type(), Protocol::Tags::node_array() );
    add_opt_to_xml( map, option, is_array );
  }
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
  cf3_assert( main_map.content.is_valid() );

  SignalFrame reply = frame.create_reply( sender );
  Map map = reply.map( Protocol::Tags::key_options() ).main_map;

  add_to_map(map, *this);

  return reply;
}

//////////////////////////////////////////////////////////////////////////////

} // XML
} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////////
