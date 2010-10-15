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
  std::string val;
  XmlNode * type_node = node.first_node(XmlTag<std::string>::type());
  int colon_pos;
  std::string protocol_str;

  if(type_node != nullptr)
    to_value(*type_node, val);
  else
    throw XmlError(FromHere(), "Could not find a stringvalue of this type.");

  colon_pos = val.find_first_of(':');

  if( colon_pos == std::string::npos ) // if ':' was not found
    throw XmlError(FromHere(), "No protocol found in URI.");

  protocol_str = val.substr(0, colon_pos);

  if(std::find(m_protocols.begin(), m_protocols.end(), protocol_str) == m_protocols.end())
    throw XmlError(FromHere(), protocol_str + ": unsupported protocol.");

  m_value = val;
}
