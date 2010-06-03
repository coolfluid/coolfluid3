#ifndef CF_Common_XML_hpp
#define CF_Common_XML_hpp

////////////////////////////////////////////////////////////////////////////////

#include <rapidxml/rapidxml.hpp>

#include "Common/CF.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////  

  /// typedef for the XmlNode
  typedef rapidxml::xml_node<> XmlNode;
  /// typedef for the XmlDoc
  typedef rapidxml::xml_document<> XmlDoc;
  /// typedef for the XmlAttribute
  typedef rapidxml::xml_attribute<> XmlAttr;
  /// typedef for the XmlMemPool
  typedef rapidxml::memory_pool<> XmlMemPool;

  /// converts the value inside the xml node to the type
  template < typename TYPE>
    void xmlstr_to_value ( XmlNode& node, TYPE& val );

  /// converts the value inside the xml node to the type
  template < typename TYPE>
    std::string value_to_xmlstr ( const TYPE& val );

  /// Converts a type to an Xml tag
  template < typename TYPE > struct XmlTag
  {
    /// xml tag for arrays
    static const char* array () { return "array"; }

    /// returns a string with the xmltag corresponding to this type
    static const char* type ();
  };


////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_XML_hpp
