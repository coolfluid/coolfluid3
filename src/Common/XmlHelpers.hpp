#ifndef CF_Common_XmlHelpers_hpp
#define CF_Common_XmlHelpers_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/filesystem/path.hpp>

#include "Common/XML.hpp"
#include "Common/NonInstantiable.hpp"
#include "Common/StringOps.hpp"
#include "Common/BasicExceptions.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Generic operations on the rapidxml structure
  /// @author Tiago Quintino
  struct Common_API XmlOps : public NonInstantiable<XmlOps>
  {

    /// prints the xml node to screen
    static void write_xml_node ( const XmlNode& node, const boost::filesystem::path& fpath );

    /// prints the xml node to screen
    static void print_xml_node ( const XmlNode& node, Uint nesting = 0 );

    /// deep copies a node into another with all the memory allocated in the second
    static void deep_copy ( const XmlNode& in, XmlNode& out );

    /// adds a node to the xml node with strings belonging to the xml document
    static XmlNode* add_node_to ( XmlNode& node, const std::string& nname,  const std::string& nvalue = std::string() );

    /// @returns the first node not part of the xml declaration
    static XmlNode* goto_doc_node ( XmlNode& node );

    /// adds an attribute to the xml node with strinss belonging to the xml document
    static XmlAttr* add_attribute_to ( XmlNode& node, const std::string& atname,  const std::string& atvalue );

    /// creates a new XmlDoc
    /// and initializes the document with Signal and Params nodes
    /// @param str string with the xml contents
    static boost::shared_ptr<XmlDoc> create_doc ();

    /// parses a xml string
    /// @param str string with the xml contents
    static boost::shared_ptr<XmlDoc> parse ( const std::string& str );

    /// parses a xml file
    /// @param fpath path to file with xml contents
    static boost::shared_ptr<XmlDoc> parse ( const boost::filesystem::path& path );

  private:

    /// Helper function that copy the names and values to the memory pool of the out node
    /// @pre assume that the nodes have been cloned before
    static void deep_copy_names_values ( const XmlNode& in, XmlNode& out );

  }; // XmlOps

  /// Helper class that extracts parameters from a XmlNode
  /// @author Tiago Quintino
  /// @todo add_param function
  /// @todo implement params converted to std::vector

  struct Common_API XmlParams
  {
    /// the xml tag used for the cfdocument node
    static const char * tag_node_doc ();
    /// the xml tag used for the params node
    static const char * tag_node_params ();
    /// the xml tag used for the signal frame node
    static const char * tag_node_frame ();
    /// the xml attribute name used for the key
    static const char * tag_attr_key ();

    /// Constructor
    /// @param node the node where the parameters will be extracted from
    /// @throw XmlError when the Params node is not found
    XmlParams ( XmlNode& node );

    /// access to the value of one parameter
    template < typename TYPE >
        TYPE get_param ( const std::string& pname ) const;

    /// access to the value of an array
    template < typename TYPE >
        std::vector<TYPE> get_array ( const std::string& pname ) const;

    /// add a key-value node to the parameters
    template < typename TYPE >
        void add_param ( const std::string& key, const TYPE& value );

    /// adds a reply frame parallel to the XmlNode passed
    XmlNode* add_reply_frame();

    /// reference to the XmlNode to retrieve params from
    XmlNode& xmlnode;
    /// reference to the XmlDoc to which the node belongs
    XmlDoc& xmldoc;
    /// pointer to the params node
    XmlNode* params;

  }; // XmlParams

  ////////////////////////////////////////////////////////////////////////////////

    template < typename TYPE >
        TYPE XmlParams::get_param ( const std::string& pname ) const
    {
      if ( params == 0 )
        throw  Common::XmlError( FromHere(), "XML node \'" + std::string(tag_node_params()) + "\' not found" );

      XmlNode* found_node = 0;
      const char * nodetype = XmlTag<TYPE>::type();

      // search for the node with correct type
      XmlNode* node = params->first_node( nodetype );
      for ( ; node; node = node->next_sibling( nodetype ) )
      {
        // search for the attribute with key
        XmlAttr* att = node->first_attribute( tag_attr_key() );
        if ( att && !pname.compare(att->value()) )
        {
          found_node = node;
          break;
        }
      }

      if ( !found_node )
        throw  Common::XmlError( FromHere(),
                                 "Did not find node of type [" + std::string(nodetype) + "]"
                                 " with \'key\' attribute  [" + pname + "]" );

      // convert xml value to TYPE
      TYPE value;
      xmlstr_to_value (*found_node, value);

      return value;
    }

////////////////////////////////////////////////////////////////////////////////

    template < typename TYPE >
        std::vector<TYPE> XmlParams::get_array ( const std::string& pname ) const
    {
      if ( params == 0 )
        throw  Common::XmlError( FromHere(), "XML node \'" + std::string(tag_node_params()) + "\' not found" );

      XmlNode* found_node = 0;
      const char * nodetype = XmlTag<TYPE>::array();

      // search for the node with correct type
      XmlNode* node = params->first_node( nodetype );
      for ( ; node; node = node->next_sibling( nodetype ) )
      {
        // search for the attribute with key
        XmlAttr* att = node->first_attribute( XmlTag<TYPE>::array() );
        if ( att && !pname.compare(att->value()) )
        {
          found_node = node;
          break;
        }
      }

      if ( !found_node )
        throw  Common::XmlError( FromHere(),
                                 "Did not find node of type [" + std::string(nodetype) + "]"
                                 " with \'key\' attribute  [" + pname + "]" );

      // convert xml value to TYPE
      std::vector<TYPE> result;
      TYPE tmp_value;
      XmlNode* elemnode = found_node->first_node();
      for ( ; elemnode ; elemnode = elemnode->next_sibling() )
      {
        xmlstr_to_value (*elemnode, tmp_value);
        result.push_back(tmp_value);
      }

      return result;
    }

    ////////////////////////////////////////////////////////////////////////////////

  template < typename TYPE >
      void XmlParams::add_param ( const std::string& key, const TYPE& value )
  {
    using namespace rapidxml;

    if ( params == 0 )
    {
      params = XmlOps::add_node_to ( xmlnode, XmlParams::tag_node_params() );
    }

    // convert TYPE to node name
    const char* node_name = xmldoc.allocate_string( XmlTag<TYPE>::type() );

    // convert value to string
    const char* value_str = xmldoc.allocate_string( value_to_xmlstr(value).c_str() );

    // creates the node
    XmlNode* node = xmldoc.allocate_node ( node_element, node_name, value_str );
    params->append_node(node);

    // convert key to xml atribute string
    const char* key_str = xmldoc.allocate_string( "key" );
    const char* keyvalue_str = xmldoc.allocate_string( key.c_str() );

    // creates the attribute
    XmlAttr* attr = xmldoc.allocate_attribute( key_str, keyvalue_str );
    node->append_attribute(attr);
  }

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_XmlHelpers_hpp
