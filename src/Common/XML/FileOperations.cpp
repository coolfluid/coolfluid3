// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cstdio>
#include <fstream>

#include "rapidxml/rapidxml_print.hpp" // includes rapidxml/rapidxml.hpp

#include "Common/Assertions.hpp"
#include "Common/BasicExceptions.hpp"

#include "Common/XML/FileOperations.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace XML {

/////////////////////////////////////////////////////////////////////////////

XmlDoc::Ptr parse_string ( const std::string& str )
{
  return parse_cstring(str.c_str(), str.length());
}

XmlDoc::Ptr parse_cstring ( const char* str, std::size_t length )
{
  using namespace rapidxml;

  cf_assert( is_not_null(str) );

  xml_document<>* xmldoc = new xml_document<>();

  char* ctext;

  if( length == 0 )
    ctext = xmldoc->allocate_string(str);
  else
    ctext = xmldoc->allocate_string(str, length + 1 );

  try
  {
    // parser trims and merges whitespaces
    xmldoc->parse< parse_no_data_nodes | parse_trim_whitespace >(ctext);

  }
  catch(...)
  {
    throw XmlError(FromHere(), std::string("The string [") + str + "] could"
                   "not be parsed.");
  }

  return XmlDoc::Ptr( new XmlDoc(xmldoc) );
}


XmlDoc::Ptr parse_file ( const boost::filesystem::path& path )
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

/////////////////////////////////////////////////////////////////////////////////

void to_file ( const XmlNode& node, const boost::filesystem::path& fpath )
{
  std::ofstream fout ( fpath.string().c_str() );

  std::string xml_as_string;

  XML::to_string( node, xml_as_string );

  fout << xml_as_string << std::endl;
}

void to_string ( const XmlNode& node, std::string& str )
{
  rapidxml::print(std::back_inserter(str), *node.content);
}

/////////////////////////////////////////////////////////////////////////////

} // XML
} // Common
} // CF
