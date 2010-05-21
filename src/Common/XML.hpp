#ifndef CF_Common_XML_hpp
#define CF_Common_XML_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/property_tree/detail/rapidxml.hpp>

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

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_XML_hpp
