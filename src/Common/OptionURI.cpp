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

void OptionURI::supported_protocol(URI::Protocol::Type protocol)
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
    throw XmlError(FromHere(), "Could not find a value for this option.");

  URI::Protocol::Type protocol = val.protocol();

  if(std::find(m_protocols.begin(), m_protocols.end(), protocol) == m_protocols.end())
    throw XmlError(FromHere(), URI::Protocol::Convert::instance().to_str(protocol) + ": unsupported protocol.");

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

