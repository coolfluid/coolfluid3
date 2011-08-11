// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

#include "rapidxml/rapidxml.hpp"

#include "Common/StringConversion.hpp"
#include "Common/XML/SignalFrame.hpp"
#include "Common/XML/Protocol.hpp"

#include "Common/XML/FileOperations.hpp"

#include "Common/OptionURI.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;

///////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

///////////////////////////////////////////////////////////////////////////////


OptionURI::OptionURI(const std::string & name, const URI & def) :
  Option(name,  def)
{
  m_restricted_list.push_back(def);
}

///////////////////////////////////////////////////////////////////////////////

OptionURI::~OptionURI()
{

}

///////////////////////////////////////////////////////////////////////////////

void OptionURI::supported_protocol(URI::Scheme::Type protocol)
{
  if(std::find(m_protocols.begin(), m_protocols.end(), protocol) == m_protocols.end())
    m_protocols.push_back(protocol);
}

///////////////////////////////////////////////////////////////////////////////

void OptionURI::configure ( XmlNode& node )
{
  URI val;
  rapidxml::xml_node<>* type_node = node.content->first_node( tag() );

  if(type_node != nullptr)
    val = from_str<URI>(type_node->value());
  else
    throw XmlError(FromHere(), "Could not find a value for this option.");

  URI::Scheme::Type protocol = val.scheme();
  std::string str;

  to_string(node, str);

  if(std::find(m_protocols.begin(), m_protocols.end(), protocol) == m_protocols.end())
    throw XmlError(FromHere(), URI::Scheme::Convert::instance().to_str(protocol) + " " + val.path() + " : unsupported protocol.");

  m_value = val;
}

//////////////////////////////////////////////////////////////////////////////

const char * OptionURI::tag() const
{
  return Protocol::Tags::type<URI>();
}

//////////////////////////////////////////////////////////////////////////////

std::string OptionURI::value_str () const
{
  return to_str( value<URI>() );
}

//////////////////////////////////////////////////////////////////////////////

std::string OptionURI::def_str () const
{
  return to_str( def<URI>() );
}

//////////////////////////////////////////////////////////////////////////////

void OptionURI::copy_to_linked_params ( const boost::any& val )
{
  BOOST_FOREACH ( void* v, this->m_linked_params )
  {
    value_type* cv = static_cast<value_type*>(v);
    try
    {
      *cv = boost::any_cast<value_type>(val);
    }
    catch(boost::bad_any_cast& e)
    {
      throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(val.type())+" to "+Common::class_name<value_type>());
    }

  }
}

//////////////////////////////////////////////////////////////////////////////

void OptionURI::set_supported_protocols( const std::vector<URI::Scheme::Type> & prots )
{
  std::vector<URI::Scheme::Type>::const_iterator it = prots.begin();
  m_protocols.clear();

  for( ; it != prots.end() ; ++it)
    supported_protocol( *it );
}

//////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

