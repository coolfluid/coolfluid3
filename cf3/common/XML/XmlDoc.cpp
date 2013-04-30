// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cstdio>

#include "rapidxml/rapidxml.hpp"

#include "common/Assertions.hpp"
#include "common/BasicExceptions.hpp"

#include "common/XML/XmlDoc.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
namespace XML {

/////////////////////////////////////////////////////////////////////////////

XmlDoc::XmlDoc(const std::string & version, const std::string & encoding):
    XmlNode(new rapidxml::xml_document<>())
{
  XmlNode decl_node(content->document()->allocate_node(rapidxml::node_declaration));

  // we first need to append the node so that it belongs to a
  // document, thus strings can be allocated for the attributes.
  content->append_node(decl_node.content);

  decl_node.set_attribute("version", version);
  decl_node.set_attribute("encoding", encoding);
}

/////////////////////////////////////////////////////////////////////////////

XmlDoc::~XmlDoc()
{
  delete content->document();
}

/////////////////////////////////////////////////////////////////////////////

XmlDoc::XmlDoc(rapidxml::xml_document<>* doc) :
    XmlNode(doc)
{
  cf3_assert( is_valid() );

  XmlNode decl_node(content->first_node());

  // if declaration node does not exist, we add it
  if( !decl_node.is_valid() || decl_node.content->type() != rapidxml::node_declaration)
  {
    decl_node.content = doc->allocate_node(rapidxml::node_declaration);
    content->prepend_node(decl_node.content);
  }

  // add information attributes if they do not exist
  if( decl_node.content->first_attribute("version") != nullptr )
    decl_node.set_attribute("version", std::string());

  if( decl_node.content->first_attribute("encoding") != nullptr )
    decl_node.set_attribute("encoding", std::string());
}

/////////////////////////////////////////////////////////////////////////////

void XmlDoc::set_version(const std::string & version)
{
  const char * ver = content->document()->allocate_string(version.c_str());

  content->first_node()->first_attribute("version")->value(ver);
}

/////////////////////////////////////////////////////////////////////////////

void XmlDoc::set_encoding(const std::string & encoding)
{
  const char * enc = content->document()->allocate_string(encoding.c_str());

  content->first_node()->first_attribute("encoding")->value(enc);
}

/////////////////////////////////////////////////////////////////////////////

std::string XmlDoc::version() const
{
  return content->first_node()->first_attribute("version")->value();
}

/////////////////////////////////////////////////////////////////////////////

std::string XmlDoc::encoding() const
{
  return content->first_node()->first_attribute("encoding")->value();
}

/////////////////////////////////////////////////////////////////////////////

} // XML
} // common
} // cf3
