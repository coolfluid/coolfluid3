#ifndef CF_Common_XML_hpp
#define CF_Common_XML_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/property_tree/detail/rapidxml.hpp>

#include "Common/CF.hpp"
#include "Common/NonInstantiable.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// typedef for the XmlNode
  typedef rapidxml::xml_node<> XmlNode;

  /// typedef for the XmlDoc
  typedef rapidxml::xml_node<> XmlDoc;

  /// Generic operations on the rapidxml structure
  /// @author Tiago Quintino
  struct Common_API XmlOps : public NonInstantiable<XmlOps>
  {
    /// prints the xml node to screen
    static void print_xml_node( const XmlNode& node );

    /// deep copies a node into another with all the memory allocated in the second
    static void deep_copy ( const XmlNode& in, XmlNode& out );

  private:

    /// Helper function that copy the names and values to the memory pool of the out node
    /// @pre assume that the nodes have been cloned before
    static void deep_copy_names_values ( const XmlNode& in, XmlNode& out );

  }; // XmlOps


  /// Helper class that extracts parameters from a XmlNode
  class Common_API XmlParams
  {

    /// Constructor
    /// @param node the node where the parameters will be extracted from
    XmlParams ( XmlNode& node ) : xml(node) {}

    XmlNode& xml;

  }; // XmlParams

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_XML_hpp
