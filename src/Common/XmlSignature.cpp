// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/URI.hpp"

#include "Common/XmlSignature.hpp"

using namespace CF::Common;

XmlSignature XmlSignature::make_signature(const XmlNode & node)
{
  return XmlSignature(node);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

XmlSignature::XmlSignature() :
    m_parent(CFNULL)
{
  boost::shared_ptr<XmlDoc> xmldoc = XmlOps::create_doc();
  m_data = XmlOps::add_node_to(*xmldoc.get(), XmlParams::tag_node_valuemap());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

XmlSignature::XmlSignature(const XmlNode & node) :
    m_parent(CFNULL)
{
  boost::shared_ptr<XmlDoc> xmldoc = XmlOps::create_doc();
  m_data = XmlOps::add_node_to(*xmldoc.get(), XmlParams::tag_node_valuemap());

  XmlOps::deep_copy(node, *m_data);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

XmlSignature::XmlSignature(XmlNode * node, XmlSignature * parent) :
    m_parent(parent),
    m_data(node)
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

XmlSignature::~XmlSignature()
{
  std::map<std::string, XmlSignature *>::iterator it = m_valuemaps.begin();

  if(m_parent == CFNULL)
  {
    // the following instruction makes the code crashing
//    m_data->document()->clear();
//    delete m_data->document();
  }

  for( ; it != m_valuemaps.end() ; it++)
    delete it->second;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void XmlSignature::put_signature(XmlNode & node) const
{
  XmlOps::deep_copy(*m_data, node);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool XmlSignature::validate(const XmlNode & node) const
{
  XmlNode * value_node = m_data->first_node();
  bool valid = true;

  while( value_node != CFNULL && valid)
  {
    std::string name = value_node->first_attribute(XmlParams::tag_attr_key())->value();
    std::string node_name = value_node->name();

    if( node_name.compare(XmlParams::tag_node_valuemap()) == 0 )
    {
      std::map<std::string, XmlSignature *>::const_iterator it = m_valuemaps.find(name);
      const XmlNode * valuemap = XmlParams::get_valuemap_from(node, name);

      if( valuemap != CFNULL )
        valid = it->second->validate( *valuemap );
      else
        valid = false;
    }
    else if( node_name.compare( XmlParams::tag_node_value() ) == 0 )
    {
      std::string type = value_node->first_node()->name();

      if(type == "bool")
        valid = XmlParams::check_value_in<bool>(node, name);
      else if(type == "integer")
        valid = XmlParams::check_value_in<int>(node, name);
      else if(type == "unsigned")
        valid = XmlParams::check_value_in<unsigned int>(node, name);
      else if(type == "real")
        valid = XmlParams::check_value_in<CF::Real>(node, name);
      else if(type == "string")
        valid = XmlParams::check_value_in<std::string>(node, name);
      else if(type == "file")
        valid = XmlParams::check_value_in<boost::filesystem::path>(node, name);
      else if(type == "uri")
        valid = XmlParams::check_value_in<URI>(node, name);
    }
    else if( node_name.compare( "array" ) == 0 )
    {
      std::string type = value_node->first_attribute(XmlParams::tag_attr_type())->value();

      if(type == "bool")
        valid = XmlParams::check_array_in<bool>(node, name);
      else if(type == "integer")
        valid = XmlParams::check_array_in<int>(node, name);
      else if(type == "unsigned")
        valid = XmlParams::check_array_in<unsigned int>(node, name);
      else if(type == "real")
        valid = XmlParams::check_array_in<CF::Real>(node, name);
      else if(type == "string")
        valid = XmlParams::check_array_in<std::string>(node, name);
      else if(type == "file")
        valid = XmlParams::check_array_in<boost::filesystem::path>(node, name);
      else if(type == "uri")
        valid = XmlParams::check_array_in<URI>(node, name);
    }

    value_node = value_node->next_sibling();
  }

  return valid;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

XmlSignature & XmlSignature::back()
{
  cf_assert( m_parent != CFNULL );
  return *m_parent;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

XmlSignature & XmlSignature::insert_valuemap(const std::string & name, const std::string & desc)
{
  cf_assert( !name.empty() );
  XmlSignature * sig;

  if( XmlParams::check_key_in(*m_data, name) )
  {
    throw ValueExists(FromHere(), "A value with name [" + name + "] already exists.");
  }

  sig = new XmlSignature(XmlParams::add_valuemap_to(*m_data, name, desc), this);

  m_valuemaps[name] = sig;

  return *sig;
}

