// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_XmlHelpers_hpp
#define CF_Common_XmlHelpers_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/filesystem/path.hpp>

#include "Common/XML.hpp"
#include "Common/NonInstantiable.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/OptionArray.hpp"

#include "Common/String/Conversion.hpp"

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
  /// @todo add_option function
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
    /// the xml tag used for the value node
    static const char * tag_node_value ();
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
    /// the key attribute value for option list
    static const char * tag_key_options();
    /// the key attribute value for property list
    static const char * tag_key_properties();

    /// Constructor
    /// @param node the node where the parameters will be extracted from
    ///        might be itself a valuemap in which case this will be detected
    /// @throw XmlError when the Params node is not found
    XmlParams ( XmlNode& node );

    /// returns the params node as reference
    /// @throw XmlError if the params node was not found
    XmlNode& get_options_node() const;

    /// access to the value of one option
    template < typename TYPE >
        TYPE get_option ( const std::string& pname ) const;

    /// access to the value of one option
    template < typename TYPE >
        TYPE get_property ( const std::string& pname ) const;

    /// access to the value of an array
    template < typename TYPE >
        std::vector<TYPE> get_array ( const std::string& pname ) const;

    /// add a key-value node to the options
    template < typename TYPE >
        void add_option ( const std::string& key, const TYPE& value,
                          const std::string& desc = std::string(), bool basic = false);

    /// add a key-value node to the options
    template < typename TYPE >
        void add_property ( const std::string& key, const TYPE& value);

    /// add an array node to the parameters
    template < typename TYPE >
        void add_array ( const std::string& key, const std::vector<TYPE>& vect, const std::string& desc = std::string(), bool basic = false);

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
    /// pointer to the valuemap of option nodes
    XmlNode* option_map;
    /// pointer to the valuemap of property nodes
    XmlNode* property_map;

  private:

    /// access to the value of one parameter
    template < typename TYPE >
        TYPE get_value_from (const XmlNode & map, const std::string& pname ) const;

    /// add a key-value node to the options
    template < typename TYPE >
        XmlNode * add_value_to (XmlNode & map, const std::string& key, const TYPE& value);

    XmlNode* add_valuemap(const char * key);

    XmlNode* seek_valuemap(const char * key);

  }; // XmlParams

  ////////////////////////////////////////////////////////////////////////////////

  template < typename TYPE >
      TYPE XmlParams::get_option ( const std::string& pname ) const
  {
    std::string str;

    XmlOps::xml_to_string(xmlnode, str);

    if ( option_map == 0 )
      throw  Common::XmlError( FromHere(), "XML node \'" + std::string(tag_node_valuemap()) + "\' not found for options\n\n" + str );

    return get_value_from<TYPE>(*option_map, pname);
  }

  //////////////////////////////////////////////////////////////////////////////

  template < typename TYPE >
      TYPE XmlParams::get_property ( const std::string& pname ) const
  {
    if ( property_map == 0 )
      throw  Common::XmlError( FromHere(), "XML node \'" + std::string(tag_node_valuemap()) + "\' not found for properties" );

    return get_value_from<TYPE>(*property_map, pname);
  }

////////////////////////////////////////////////////////////////////////////////

    template < typename TYPE >
        std::vector<TYPE> XmlParams::get_array ( const std::string& pname ) const
    {
      if ( option_map == 0 )
        throw  Common::XmlError( FromHere(), "XML node \'" + std::string(tag_node_valuemap()) + "\' not found for options");

      XmlNode* found_node = 0;
      const char * nodetype = XmlTag<TYPE>::array();

      // search for the node with correct type
      XmlNode* node = option_map->first_node( nodetype );
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
        to_value (*elemnode, tmp_value);
        result.push_back(tmp_value);
      }

      return result;
    }

    ////////////////////////////////////////////////////////////////////////////////

    template < typename TYPE >
        void XmlParams::add_option ( const std::string& key, const TYPE& value, const std::string& desc, bool basic)
    {
      if ( option_map == 0 )
        option_map = add_valuemap( tag_key_options() );

      XmlNode * node = add_value_to(*option_map, key, value);

      XmlOps::add_attribute_to(*node, "mode", basic ? "basic" : "adv" );

      // add the description if any
      if(!desc.empty())
      {
        const char* desc_str = xmldoc.allocate_string( tag_attr_descr() );
        const char* descvalue_str = xmldoc.allocate_string( desc.c_str() );

        // create the attribute
        XmlAttr* desc_attr = xmldoc.allocate_attribute( desc_str, descvalue_str );
        node->append_attribute(desc_attr);
      }
    }

    ////////////////////////////////////////////////////////////////////////////////

    template < typename TYPE >
        void XmlParams::add_property ( const std::string& key, const TYPE& value)
    {
      if ( property_map == 0 )
        property_map = add_valuemap( tag_key_properties() );

      add_value_to(*property_map, key, value);
    }

    ////////////////////////////////////////////////////////////////////////////////

  template < typename TYPE >
      void XmlParams::add_array ( const std::string& key, const std::vector<TYPE>& vect, const std::string& desc, bool basic)
  {
    if ( option_map == 0 )
      option_map = add_valuemap( tag_key_options() );

    // create the "array" node and append it to the map
    XmlNode* node = XmlOps::add_node_to(*option_map, XmlTag<TYPE>::array());

    // add "key", "size" and "type" attributes to "array" node
	// note : the size of the array has to be explicitly cast to CF::Uint or
	// MSVC will consider the value to be of type "unsigned __int64" 
	// (defined by Microsoft) and the linking will fail because 
	// String::to_str<unsigned __int64>() is not defined.
    XmlOps::add_attribute_to(*node, tag_attr_key(), key);
	XmlOps::add_attribute_to(*node, tag_attr_size(), String::to_str( (CF::Uint) vect.size() ));
    XmlOps::add_attribute_to(*node, tag_attr_type(), XmlTag<TYPE>::type());

    for(size_t i = 0 ; i < vect.size() ; i++)
      XmlOps::add_node_to(*node, "e", from_value(vect[i]));
  }

  ////////////////////////////////////////////////////////////////////////////////

  template < typename TYPE >
      TYPE XmlParams::get_value_from (const XmlNode & map, const std::string& pname ) const
  {
    XmlNode* found_node = 0;
    const char * nodetype = XmlTag<TYPE>::type();

    // search for the node with correct type
    XmlNode* node = map.first_node( "value" );
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
    return to_value<TYPE>(*found_node);
  }

  ////////////////////////////////////////////////////////////////////////////////

  /// add a key-value node to the options
  template < typename TYPE >
      XmlNode * XmlParams::add_value_to (XmlNode & map, const std::string& key, const TYPE& value)
  {
    std::string type_name = XmlTag<TYPE>::type();
    std::string value_str = from_value(value); // convert value to string

    // create "value" node and append it to the valuemap
    XmlNode* node = XmlOps::add_node_to(map, tag_node_value());

    // create the type node (with option value) and append it to the "value" node
    XmlOps::add_node_to(*node, type_name, value_str);

    // add "key" attribute
    XmlOps::add_attribute_to(*node, tag_attr_key(), key);

    return node;
  }

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_XmlHelpers_hpp
