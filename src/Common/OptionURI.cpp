// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/OptionURI.hpp"
#include "Common/XmlHelpers.hpp"

using namespace CF::Common;

OptionURI::OptionURI(const std::string & name, const std::string & desc,
                     const URI & def) :
  Option(name, desc, def)
{

}

///////////////////////////////////////////////////////////////////////////////

OptionURI::~OptionURI()
{

}

///////////////////////////////////////////////////////////////////////////////

void OptionURI::supported_protocol(const std::string & protocol)
{
  if(std::find(m_protocols.begin(), m_protocols.end(), protocol) != m_protocols.end())
    m_protocols.push_back(protocol);
}

///////////////////////////////////////////////////////////////////////////////

void OptionURI::configure ( XmlNode& node )
{
  URI val;
  XmlNode * type_node = node.first_node(XmlTag<URI>::type());

  if(type_node != nullptr)
    to_value(*type_node, val);
  else
    throw XmlError(FromHere(), "Could not find a string value of this type.");


//  size_t colon_pos = val.find_first_of(':');

//  if( colon_pos == std::string::npos ) // if ':' was not found
//    throw XmlError(FromHere(), "No protocol found in URI.");

  throw NotImplemented(FromHere(), "adapt to the new protocol system");
//  std::string protocol_str = val.protocol(); //val.substr(0, colon_pos);

//  if(!m_protocols.empty())
//  {
//    if(std::find(m_protocols.begin(), m_protocols.end(), protocol_str) == m_protocols.end())
//      throw XmlError(FromHere(), protocol_str + ": unsupported protocol.");
//  }

  m_value = val;
}

//////////////////////////////////////////////////////////////////////////////

void OptionURI::copy_to_linked_params ( const boost::any& val )
{
  BOOST_FOREACH ( void* v, this->m_linked_params )
  {
    value_type* cv = static_cast<value_type*>(v);
    *cv = boost::any_cast<value_type>(val);
  }
}

//////////////////////////////////////////////////////////////////////////////

