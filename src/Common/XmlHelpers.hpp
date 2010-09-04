#ifndef CF_Common_XmlHelpers_hpp
#define CF_Common_XmlHelpers_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/filesystem/path.hpp>

#include "Common/XML.hpp"
#include "Common/NonInstantiable.hpp"
#include "Common/StringOps.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/OptionArray.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  class CPath;

  /// Generic operations on the rapidxml structure
  /// @author Tiago Quintino
  struct Common_API XmlOps : public NonInstantiable<XmlOps>
  {

    /// prints the xml node to screen
    static void write_xml_node ( const XmlNode& node, const boost::filesystem::path& fpath );

    /// prints the xml node to screen
    static void print_xml_node ( const XmlNode& node, Uint nesting = 0 );

    static void xml_to_string ( const XmlNode& node, std::string& str );

    /// deep copies a node into another with all the memory allocated in the second
    static void deep_copy ( const XmlNode& in, XmlNode& out );

    /// adds a node to the xml node with strings belonging to the xml document
    static XmlNode* add_node_to ( XmlNode& node, const std::string& nname,  const std::string& nvalue = std::string() );

    /// adds an attribute to the xml node with strinss belonging to the xml document
    static XmlAttr* add_attribute_to ( XmlNode& node, const std::string& atname,  const std::string& atvalue );

    /// @returns the first node not part of the xml declaration
    static XmlNode* goto_doc_node ( XmlNode& node );

    /// @returns the first frame node inside a doc node
    static XmlNode* first_frame_node ( XmlNode& node );

    /// Adds a signal frame within the XmlNode passed ( typically a doc ).
    /// It will set automatically the type attribute.
    static XmlNode* add_signal_frame( XmlNode& node, const std::string & target, const CPath & sender,  const CPath & receiver, bool userTrans);

    /// adds a reply frame parallel to the XmlNode passed.
    /// It will set automatically the type attrribute.
    /// It will try to set the receiver and the target from the signal to which is answering
    static XmlNode* add_reply_frame( XmlNode& node );

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
    /// the xml tag used for the signal node
    static const char * tag_node_signal ();
    /// the xml tag used for the reply node
    static const char * tag_node_reply ();
    /// the xml tag used for the params node
    static const char * tag_node_valuemap ();
    /// the xml tag used for the signal frame node
    static const char * tag_node_frame ();
    /// the xml attribute name used for the key
    static const char * tag_attr_key ();
    /// the xml attribute name used for the type (array options)
    static const char * tag_attr_type ();
    /// the xml attribute name used for the size (array options)
    static const char * tag_attr_size ();
    /// the xml attribute name used for the description
    static const char * tag_attr_descr ();
    /// the xml attribute name used for the sender UUID
    static const char * tag_attr_clientid();
    /// the xml attribute name used for the frame UUID
    static const char * tag_attr_frameid();

    /// Constructor
    /// @param node the node where the parameters will be extracted from
    ///        might be itself a valuemap in which case this will be detected
    /// @throw XmlError when the Params node is not found
    XmlParams ( XmlNode& node );

    /// returns the params node as reference
    /// @throw XmlError if the params node was not found
    XmlNode& get_params_node() const;

    /// access to the value of one parameter
    template < typename TYPE >
        TYPE get_param ( const std::string& pname ) const;

    /// access to the value of an array
    template < typename TYPE >
        std::vector<TYPE> get_array ( const std::string& pname ) const;

    /// add a key-value node to the parameters
    template < typename TYPE >
        void add_param ( const std::string& key, const TYPE& value );

    /// add an array node to the parameters
    template < typename TYPE >
        void add_array ( const std::string& key, const std::vector<TYPE>& vect);

    /// Sets UUID attribute to the first frame found
    /// If the attribute does not exists, it is created; otherwise its value
    /// is replaced.
    /// @param uuid UUID to set
    /// @throw XmlError if no frame was found
    void set_clientid(const std::string & uuid);

    /// Gets UUID attribute of the first frame found
    /// @return The UUID, or an empty string if not found.
    /// @throw XmlError if no frame was found
    std::string get_clientid() const;

    std::string get_frameid() const;

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
        throw  Common::XmlError( FromHere(), "XML node \'" + std::string(tag_node_valuemap()) + "\' not found" );

      XmlNode* found_node = 0;
      const char * nodetype = XmlTag<TYPE>::type();

      // search for the node with correct type
      XmlNode* node = params->first_node( "value" );
      for ( ; node; node = node->next_sibling( "value" ) )
      {
        // search for the attribute with key
        XmlAttr* att = node->first_attribute( tag_attr_key() );
        if ( att && !pname.compare(att->value()) )
        {
          XmlNode* type_node = node->first_node( XmlTag<TYPE>::type() );

          if( type_node )
          {
            found_node = type_node;
            break;
          }
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
        throw  Common::XmlError( FromHere(), "XML node \'" + std::string(tag_node_valuemap()) + "\' not found" );

      XmlNode* found_node = 0;
      const char * nodetype = XmlTag<TYPE>::array();

      // search for the node with correct type
      XmlNode* node = params->first_node( nodetype );
      for ( ; node; node = node->next_sibling( nodetype ) )
      {
        // search for the attribute with key
        XmlAttr* att = node->first_attribute( XmlParams::tag_attr_key() );
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
      params = XmlOps::add_node_to ( xmlnode, XmlParams::tag_node_valuemap() );
    }

    // convert TYPE to node name
    const char* type_name = xmldoc.allocate_string( XmlTag<TYPE>::type() );

    // convert value to string
    const char* value_str = xmldoc.allocate_string( value_to_xmlstr(value).c_str() );

    // creates the node
    //XmlNode* node = xmldoc.allocate_node ( node_element, node_name, value_str );
    XmlNode* node = xmldoc.allocate_node ( node_element, "value");
    params->append_node(node);

    // create the type node
    XmlNode* type_node = xmldoc.allocate_node ( node_element, type_name, value_str );
    node->append_node(type_node);

    // convert key to xml atribute string
    const char* key_str = xmldoc.allocate_string( "key" );
    const char* keyvalue_str = xmldoc.allocate_string( key.c_str() );

    // creates the attribute
    XmlAttr* attr = xmldoc.allocate_attribute( key_str, keyvalue_str );
    node->append_attribute(attr);
  }

  ////////////////////////////////////////////////////////////////////////////////

  template < typename TYPE >
      void XmlParams::add_array ( const std::string& key, const std::vector<TYPE>& vect)
  {
    using namespace rapidxml;

    if ( params == 0 )
    {
      params = XmlOps::add_node_to ( xmlnode, XmlParams::tag_node_valuemap() );
    }

    // convert TYPE to node name
    const char* node_name = xmldoc.allocate_string( XmlTag<TYPE>::array() );

    // convert value to string


    // creates the node
    XmlNode* node = xmldoc.allocate_node ( node_element, node_name, "" );
    params->append_node(node);

    // convert key to xml atribute string
    const char* key_str = xmldoc.allocate_string( "key" );
    const char* keyvalue_str = xmldoc.allocate_string( key.c_str() );

    // create the size attribute
    const char* size_str = xmldoc.allocate_string( "size" );
    const char* sizevalue_str = xmldoc.allocate_string( StringOps::to_str(vect.size()).c_str() );

    // creates the attribute
    XmlAttr* attr = xmldoc.allocate_attribute("type", XmlTag<TYPE>::type());
    node->append_attribute(attr);

    attr = xmldoc.allocate_attribute( key_str, keyvalue_str );
    node->append_attribute(attr);

    attr = xmldoc.allocate_attribute( size_str, sizevalue_str );
    node->append_attribute(attr);

    for(size_t i = 0 ; i < vect.size() ; i++)
    {
      const char* value_str = xmldoc.allocate_string( value_to_xmlstr(vect[i]).c_str() );
      XmlNode * itemNode = xmldoc.allocate_node ( node_element, "e", value_str );
      node->append_node(itemNode);
    }
  }

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_XmlHelpers_hpp
