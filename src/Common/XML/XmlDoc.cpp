// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cstdio>

#include "rapidxml/rapidxml.hpp"

#include "Common/BasicExceptions.hpp"

#include "Common/XML/XmlDoc.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
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
  cf_assert( is_valid() );

  XmlNode decl_node(content->first_node());

  // if declaration node does not exist, we add it
  if( decl_node.is_valid() || decl_node.content->type() != rapidxml::node_declaration)
  {
    decl_node.content = doc->allocate_node(rapidxml::node_declaration);
    content->append_node(decl_node.content);
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

XmlDoc::Ptr XmlDoc::parse_string ( const std::string& str )
{
  using namespace rapidxml;

  xml_document<>* xmldoc = new xml_document<>();

  char* ctext = xmldoc->allocate_string(str.c_str());

  // parser trims and merges whitespaces
  xmldoc->parse< parse_no_data_nodes |
                 parse_trim_whitespace /*|
                 parse_normalize_whitespace*/ >(ctext);

  return boost::shared_ptr<XmlDoc>( new XmlDoc(xmldoc) );
}

//////////////////////////////////////////////////////////////////////////////

XmlDoc::Ptr XmlDoc::parse_file ( const boost::filesystem::path& path )
{
  using namespace rapidxml;

  xml_document<>* xmldoc = new xml_document<>();

  std::string filepath = path.string();
  FILE *filep = fopen( filepath.c_str(), "rb" );

  if (filep == NULL)
    throw FileSystemError(FromHere(), "Unable to open file [" + filepath + "]" );

  fseek(filep,0,SEEK_END);                  // go to end
  Uint length = ftell(filep);               // get position at end (length)

  if (!length)
    throw FileSystemError(FromHere(), "File [" + filepath + "] is empty" );

  fseek(filep, 0, SEEK_SET);                  // go to beginning

  char* buffer = xmldoc->allocate_string( 0, length );  // allocate buffer directly inside the xmldoc

  size_t rs = fread(buffer,length, 1, filep);           // read into buffer
  if (!rs)
    throw FileSystemError(FromHere(), "Error while reading file [" + filepath + "]" );

  fclose(filep);                             // close file

  // parser trims and merges whitespaces
  xmldoc->parse< parse_no_data_nodes |
                 parse_trim_whitespace /*|
                 parse_normalize_whitespace*/ >(buffer);

  return boost::shared_ptr<XmlDoc>( new XmlDoc(xmldoc) );
}


/////////////////////////////////////////////////////////////////////////////

} // XML
} // Common
} // CF
