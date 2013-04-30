// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cstdio>
#include <fstream>

#include "rapidxml/rapidxml_print.hpp" // includes rapidxml/rapidxml.hpp

#include "common/Assertions.hpp"
#include "common/BasicExceptions.hpp"

#include "common/XML/FileOperations.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
namespace XML {

/////////////////////////////////////////////////////////////////////////////

boost::shared_ptr<XmlDoc> parse_string ( const std::string& str )
{
  return parse_cstring(str.c_str(), str.length());
}

boost::shared_ptr<XmlDoc> parse_cstring ( const char* str, std::size_t length )
{
  using namespace rapidxml;

  cf3_assert( is_not_null(str) );

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
  catch(rapidxml::parse_error& e)
  {
    throw XmlError(FromHere(), std::string("Parse error: ") + e.what() + std::string(" in ") + e.where<char>());// + "\nfor string [" + str + "]");
  }
  catch(...)
  {
    throw XmlError(FromHere(), "Unknown error when parsing XML string");
  }

  return boost::shared_ptr<XmlDoc>( new XmlDoc(xmldoc) );
}


boost::shared_ptr<XmlDoc> parse_file ( const URI& file )
{
  using namespace rapidxml;

  xml_document<>* xmldoc = new xml_document<>();

  std::string filepath = file.path();
  FILE *filep = fopen( filepath.c_str(), "rb" );

  if (filep == NULL)
    throw FileSystemError(FromHere(), "Unable to open file [" + filepath + "]" );

  fseek(filep,0,SEEK_END);                  // go to end
  Uint length = ftell(filep);               // get position at end (length)

  if (!length)
    throw FileSystemError(FromHere(), "File [" + filepath + "] is empty" );

  fseek(filep, 0, SEEK_SET);                  // go to beginning

  char* buffer = xmldoc->allocate_string( 0, length + 1 );  // allocate buffer directly inside the xmldoc

  size_t rs = fread(buffer, 1, length, filep);           // read into buffer
  if (!rs)
    throw FileSystemError(FromHere(), "Error while reading file [" + filepath + "]" );

  buffer[rs] = '\0';

  fclose(filep);                             // close file

  // parser trims and merges whitespaces
  xmldoc->parse< parse_no_data_nodes |
                 parse_trim_whitespace /*|
                 parse_normalize_whitespace*/ >(buffer);

  return boost::shared_ptr<XmlDoc>( new XmlDoc(xmldoc) );
}

/////////////////////////////////////////////////////////////////////////////////

void to_file ( const XmlNode& node, const URI& file )
{
  std::ofstream fout ( file.path().c_str() );

  std::string xml_as_string;

  XML::to_string( node, xml_as_string );

  fout << xml_as_string << std::endl;
}

void to_string ( const XmlNode& node, std::string& str )
{
  str.clear(); // back_inserter appends, so we need to clear the string before
  rapidxml::print(std::back_inserter(str), *node.content);
}

/////////////////////////////////////////////////////////////////////////////

} // XML
} // common
} // cf3
