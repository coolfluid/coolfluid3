// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_XML_FileOperations_hpp
#define cf3_common_XML_FileOperations_hpp

////////////////////////////////////////////////////////////////////////////

#include "common/URI.hpp"
#include "common/XML/XmlDoc.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
namespace XML {

/// Parses a XML string
/// @param str String with the XML contents
/// @return Returns a shared pointer with the built XML document.
boost::shared_ptr<XmlDoc> parse_string ( const std::string& str );

/// Parses a XML file
/// @param fpath Path to file with xml contents
/// @return Returns a shared pointer with the built XML document.
/// @throw FileSystemError If the file cannot be read.
boost::shared_ptr<XmlDoc> parse_file ( const URI& path );

/// Parses a XML C-string
/// @param str String with the XML contents, cannot be null.
/// @param length The length of the string
/// @return Returns a shared pointer with the built XML document.
/// @throw XmlError If the string could not be parsed.
boost::shared_ptr<XmlDoc> parse_cstring ( const char * str, std::size_t length = 0 );

/// Writes the provided XML node to a file.
/// @param node The node to write.
/// @param fpath The file path to which the node has to be written.
void to_file ( const XmlNode& node, const URI& fpath);

/// Writes the provided XML node to a string.
/// @param str The string to which the node has to be written.
/// @param node The node to write.
void to_string ( const XmlNode& node, std::string& str );

} // XML
} // common
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_XML_FileOperations_hpp
