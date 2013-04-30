// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_XML_XmlDoc_hpp
#define cf3_common_XML_XmlDoc_hpp

////////////////////////////////////////////////////////////////////////////

#include <boost/shared_ptr.hpp>

#include "common/XML/XmlNode.hpp"

namespace rapidxml
{
  template<class Ch> class xml_document;
}

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
namespace XML {

////////////////////////////////////////////////////////////////////////////

/// Represents a XML document.
/// A XML document is a special XML node that manages the memory.
/// This class uses the Pimpl idiom. It hides the real XML implementation,
/// which should never be used directly. @n
/// This memory management is implementation-dependent and should never be
/// used directly.
/// @author Quentin Gasper
class Common_API XmlDoc : public XmlNode
{

public:

  /// Creates an empty document.
  XmlDoc(const std::string & version = std::string(),
         const std::string & encoding = std::string());

  /// Builds a document from an existing document.
  /// A declaration node is added if it is not present in the provided document.
  /// @param doc Document on which the new @c XmlDoc object is based.
  XmlDoc(rapidxml::xml_document<char>* doc);

  /// Destroys the XML document.
  /// All allocated memory is freed.
  ~XmlDoc();

  /// Sets the version of the XML document.
  /// The version is stored as attribute in the declaration node ("<?xml ...>").
  /// If the node or the attribute does not exist yet, it is created.
  /// @param version Version in string format.
  void set_version(const std::string & version);

  /// Sets the encoding of the XML document.
  /// The encoding is stored as attribute in the declaration node ("<?xml ...>").
  /// If the node or the attribute does not exist yet, it is created.
  /// @param encoding Encoding in string format (i.e. "UTF-8").
  void set_encoding(const std::string & encoding);

  /// Gives the document version.
  /// @return Returns the document version. The string returned may be empty
  /// if no version was set.
  std::string version() const;

  /// Gives the document encoding.
  /// @return Returns the document encoding. The string returned may be empty
  /// if no encoding was set.
  std::string encoding() const;

}; // XmlDoc

////////////////////////////////////////////////////////////////////////////

} // XML
} // common
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_XML_XmlDoc_hpp
