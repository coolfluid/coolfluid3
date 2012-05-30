// Copyright  (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3  (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_XML_Protocol_hpp
#define cf3_common_XML_Protocol_hpp

////////////////////////////////////////////////////////////////////////////

#include "common/URI.hpp"
#include "common/XML/XmlDoc.hpp"

/////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
namespace XML {

/////////////////////////////////////////////////////////////////////////////////

  /// Provides functions for basic XML protocol manipulations.
  /// @author Tiago Quintino (initial author)
  /// @author Quentin Gasper
  class Common_API Protocol : public NonInstantiable<Protocol>
  {
  public:

    /// Provides tags related to the XML protocol
    /// The first word of each function indicates the place it should be used in
    /// the XML tree:
    /// @li @c node_xxx : gives a node name
    /// @li @c attr_xxx : gives an attribute name
    /// @li @c key_xxx : gives a value for 'key' attribute
    /// @li @c node_type_xxx : gives a value for 'type' key in a 'frame' node
    /// @author Quentin Gasper
    class Tags : public NonInstantiable<Tags>
    {
    public:

      /// @returns Returns the name for attribute 'delimiter' of arrays.
      static const char * attr_array_delimiter ();
      /// @returns Returns the name for attribute 'size' of arrays.
      static const char * attr_array_size ();
      /// @returns Returns the name for attribute 'type' of arrays.
      static const char * attr_array_type ();


      /// @returns Returns the name for attribute that maintains the client UUID.
      static const char * attr_clientid ();
      /// @returns Returns the name for attribute that maintains a description.
      static const char * attr_descr ();
      /// @returns Returns the name for attribute that maintains the frame UUID.
      static const char * attr_frameid ();
      /// @returns Returns the name for attribute that maintains a name (the key).
      static const char * attr_key ();
      /// @returns Returns the name for attribute that maintains a pretty name.
      static const char * attr_pretty_name ();
      
      static const char * key_restricted_values ();

      /// @returns Returns the name for attribute that maintains protocols for
      /// URI options/arrays.
      static const char * attr_uri_schemes ();

      /// @return Returns the key value for option maps.
      static const char * key_options ();
      /// @return Returns the key value for property maps.
      static const char * key_properties ();
      /// @return Returns the key value for signal maps.
      static const char * key_signals ();

      /// @return Returns the node name for XML document.
      static const char * node_doc ();
      /// @return Returns the node name for frames.
      static const char * node_frame ();
      /// @return Returns the node name for maps.
      static const char * node_map ();
      /// @return Returns the node name for values.
      static const char * node_value ();
      /// @return Returns the node name for arrays.
      static const char * node_array ();

      /// @return Returns the type for reply frames.
      static const char * node_type_reply ();
      /// @return Returns the type for signal frames.
      static const char * node_type_signal ();
    }; // Tags

    /// Searches for a document node after the XML declaration.
    /// @param The node under which to search.
    /// @return The first node not part of the XML declaration.
    /// @throw XmlError If a such node was not found.
    static XmlNode goto_doc_node ( const XmlNode& node );

    /// Searches for a frame node.
    /// @param node Node under which to search.
    /// @returns The first frame node inside a doc node, or a invalid
    /// node if no frame was found.
    static XmlNode first_frame_node ( const XmlNode& node );

    /// Adds a signal frame within the XmlNode passed ( typically a doc ).
    /// It will set automatically the type attribute.
    /// @param node The node in which to add the new frame.
    /// @param target The target (signal name).
    /// @param sender Path to the sender component.
    /// @param receiver Path to the receiver component.
    /// @param user_transaction If @c true, the signal was initiated from a user
    /// transaction.
    /// @return Returns the new signal frame node.
    static XmlNode add_signal_frame( XmlNode& node, const std::string & target,
                                     const URI & sender, const URI & receiver,
                                     bool user_transaction );

    /// Adds a reply frame parallel to the XmlNode passed.
    /// It will set automatically the type attrribute.
    /// It will try to set the receiver and the target from the signal to which
    /// it is answering.
    /// @param node Signal frame node to which a reply has to added.
    /// @return Returns the new reply frame node.
    static XmlNode add_reply_frame( XmlNode& node );

    /// Creates a new XmlDoc.
    /// @return Returns a shared pointer with the new document.
    static boost::shared_ptr<XmlDoc> create_doc ();

  }; // Protocols

////////////////////////////////////////////////////////////////////////////////

} // XML
} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_XML_Protocol_hpp
