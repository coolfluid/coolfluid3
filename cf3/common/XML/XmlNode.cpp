// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "rapidxml/rapidxml.hpp"

#include "common/BasicExceptions.hpp"
#include "common/Log.hpp"

#include "common/XML/XmlDoc.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
namespace XML {

/////////////////////////////////////////////////////////////////////////////

XmlNode::XmlNode () :
    content(nullptr)
{}

XmlNode::~XmlNode () {}

/////////////////////////////////////////////////////////////////////////////

XmlNode::XmlNode (rapidxml::xml_node<> * impl) :
    content(impl)
{

}

/////////////////////////////////////////////////////////////////////////////

XmlNode XmlNode::add_node ( const std::string & name, const std::string & value ) const
{
  cf3_assert( is_valid() );
  cf3_assert( is_not_null(content->document()) );

  rapidxml::xml_document<>& doc = *content->document();
  rapidxml::xml_node<>* node = doc.allocate_node( rapidxml::node_element,
                                      doc.allocate_string( name.c_str()),
                                      doc.allocate_string( value.c_str()) );
  content->append_node(node);

  return node;
}

/////////////////////////////////////////////////////////////////////////////

void XmlNode::set_attribute ( const std::string & name, const std::string & value )
{
  cf3_assert( is_valid() );
  cf3_assert( is_not_null(content->document()) );

  rapidxml::xml_document<>& doc = *content->document();
  rapidxml::xml_attribute<>* attr(nullptr);
  attr = content->first_attribute( name.c_str() );
  const char * the_value = doc.allocate_string( value.c_str() );

  if( attr == nullptr )
  {
    attr = doc.allocate_attribute( doc.allocate_string(name.c_str()), the_value );
    content->append_attribute(attr);
  }
  else
    attr->value( the_value );
}

/////////////////////////////////////////////////////////////////////////////

std::string XmlNode::attribute_value ( const std::string & name ) const
{
  rapidxml::xml_attribute<> * attr = content->first_attribute( name.c_str() );

  return is_not_null(attr) ? attr->value() : std::string();
}

/////////////////////////////////////////////////////////////////////////////

void XmlNode::set_name ( const char * name )
{
  cf3_assert( is_valid() );

  content->name(content->document()->allocate_string(name));
}

/////////////////////////////////////////////////////////////////////////////

void XmlNode::set_value ( const char * value )
{
  cf3_assert( is_valid() );

  content->value(content->document()->allocate_string(value));
}

/////////////////////////////////////////////////////////////////////////////

bool XmlNode::is_valid() const
{
  return is_not_null(content);
}

/////////////////////////////////////////////////////////////////////////////

void XmlNode::deep_copy ( XmlNode& out ) const
{
  cf3_assert( is_valid() );
  cf3_assert( out.is_valid() );

  rapidxml::xml_document<>& doc = *out.content->document();

  doc.clone_node(content, out.content);

  deep_copy_names_values(*this, out);
}

/////////////////////////////////////////////////////////////////////////////

void XmlNode::deep_copy_names_values ( const XmlNode& in, XmlNode& out ) const
{
  out.set_name(in.content->name());
  out.set_value(in.content->value());

  // copy names and values of the attributes
  rapidxml::xml_attribute<> * iattr = in.content->first_attribute();
  rapidxml::xml_attribute<> * oattr = out.content->first_attribute();

  for ( ; iattr != nullptr ; iattr = iattr->next_attribute(), oattr = oattr->next_attribute() )
  {
    oattr->name( oattr->document()->allocate_string(iattr->name()) );
    oattr->value( oattr->document()->allocate_string(iattr->value()) );
  }

  // copy names and values of the child nodes
  XmlNode inode(in.content->first_node());
  XmlNode onode(out.content->first_node());

  for ( ; inode.is_valid() ; inode.content = inode.content->next_sibling(),
        onode.content = onode.content->next_sibling() )
  {

    deep_copy_names_values( inode, onode );
  }
}

/////////////////////////////////////////////////////////////////////////////////

void XmlNode::print ( Uint nesting ) const
{
  std::string nest_str (nesting, '+');
  rapidxml::xml_attribute<>* attr;
  XmlNode itr;

  CFinfo << nest_str
      << " Node \'" << content->name() << "\' [" << content->value() << "]\n";

  for (attr = content->first_attribute(); attr != nullptr ; attr = attr->next_attribute())
  {
    CFinfo << nest_str
        << " - attribute \'" << attr->name() << "\' [" << attr->value() << "]\n";
  }

  for (itr.content = content->first_node(); itr.is_valid() ; itr.content = itr.content->next_sibling() )
  {
    itr.print ( nesting+1 );
  }
}

/////////////////////////////////////////////////////////////////////////////////

} // XML
} // common
} // cf3
