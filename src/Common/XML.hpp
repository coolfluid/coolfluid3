#ifndef CF_Common_XML_hpp
#define CF_Common_XML_hpp

////////////////////////////////////////////////////////////////////////////////

#include <rapidxml/rapidxml.hpp>

#include "Common/CF.hpp"

#include "Common/CommonAPI.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////  

/// typedef for the XmlBase
typedef rapidxml::xml_base<> XmlBase;
/// typedef for the XmlDoc
typedef rapidxml::xml_document<> XmlDoc;
/// typedef for the XmlNode
typedef rapidxml::xml_node<> XmlNode;
/// typedef for the XmlAttribute
typedef rapidxml::xml_attribute<> XmlAttr;
/// typedef for the XmlMemPool
typedef rapidxml::memory_pool<> XmlMemPool;

/// Converts a type to an Xml tag
template < typename TYPE > struct XmlTag
{
  /// xml tag for arrays
  static const char* array () { return "array"; }

  /// returns a string with the xmltag corresponding to this type
  static const char* type ();
};

/// converts the value inside the xml node to the type
template < typename T>
Common_API void to_value (XmlBase& node, T& val);

template < typename T>
Common_API T to_value (XmlBase& node);

/// converts the value inside the xml node to the type
template < typename T>
Common_API std::string from_value (const T& val);


////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_XML_hpp
