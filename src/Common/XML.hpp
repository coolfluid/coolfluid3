#ifndef CF_Common_XML_hpp
#define CF_Common_XML_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/property_tree/detail/rapidxml.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/CF.hpp"
#include "Common/NonInstantiable.hpp"

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
  struct Common_API XmlParser
  {
    /// Constructor
    /// @param str string with the xml contents
    XmlParser ( const std::string& str );

    /// Constructor
    /// @param fpath path to file with xml contents
    XmlParser ( const boost::filesystem::path& path );

    /// @retuns the parsed Xml
    XmlNode& getXml ();

    /// storage of the xmldoc object
    boost::shared_ptr<XmlDoc> xmldoc;

  }; // XmlParams


  /// Helper class that extracts parameters from a XmlNode
  struct Common_API XmlParams
  {
    /// Constructor
    /// @param node the node where the parameters will be extracted from
    /// @throw XmlError when the Params node is not found
    XmlParams ( XmlNode& node );

    /// storage of the XmlNode to retrieve params from
    XmlNode& xml;

    /// pointer to the params node
    XmlNode* params;

  }; // XmlParams

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_XML_hpp
