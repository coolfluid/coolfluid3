// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_XML_XmlNode_hpp
#define cf3_common_XML_XmlNode_hpp

////////////////////////////////////////////////////////////////////////////

#include "common/CF.hpp"
#include "common/CommonAPI.hpp"

/// @brief external library used for %XML parsing
namespace rapidxml
{
  template<class Ch> class xml_node;
}

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/// @brief Classes that implement the %XML protocol for use in %COOLFluiD
namespace XML {

////////////////////////////////////////////////////////////////////////////

class XmlDoc;

////////////////////////////////////////////////////////////////////////////

/// Represents a XML node.
/// This class uses the Pimpl idiom. Each object is only a wrapper for an
/// object of an underlying XML implementation. This class provides
/// functions that ease some XML manipulation, like adding a node or
/// adding/modifying an attribute, that need memory allocation.@n
/// A XmlNode can be easily written to a file, converted to a string
/// or printed to screen (not in XML format) using COOLFluiD logging facility.

/// @author Quentin Gasper
class Common_API XmlNode
{

public: // methods

  /// Pointer to the underlying XML implementation.
  rapidxml::xml_node<char> * content;

  /// Default constructor.
  /// Builds an invalid node.
  XmlNode ();

  /// Builds a node from an existing one.
  /// @param impl The node.
  XmlNode(rapidxml::xml_node<char> * impl);

  /// Destructor.
  /// The node is removed from its parent and all its child nodes and attributes
  /// are removed.
  ~XmlNode ();

  /// Creates and add a node to this node with a value, if specified.
  /// @param name The name of the new name.
  /// @param value The value of the new node. If empty, this parameter is
  /// ignored.
  /// @return Returns the new node.
  XmlNode add_node ( const std::string & name, const std::string & value = std::string() ) const;

  /// Sets an attribute of this node to a specified value.
  /// If an attribute with this name already exists, it is modified. If
  /// not, it is created.
  /// @param name The name of the new name.
  /// @param value The value of the new node.
  /// @return Returns the new attribute.
  void set_attribute ( const std::string & name, const std::string & value );

  /// Gets the value of an attribute.
  /// @param name Attribute name.
  /// @return Returns the value of the attribute or an empty string of the
  /// attribute was not found.
  std::string attribute_value ( const std::string & name ) const;

  /// Sets a new name.
  /// @param name The new name
  void set_name ( const char * name );

  /// Sets a new value.
  /// @param name The new name
  void set_value ( const char * value );

  /// Checks if the this node is valid.
  /// A node is valid if the its internal pointer to the underlying XML
  /// implementation is valid.
  /// @return Returns @c true if the node is valid, otherwise returns @c false.
  bool is_valid () const;

  /// Prints the provided XML node to the logging facility.
  /// @param node The node to print.
  /// @param nesting Indentation between a node and its attributes or sub-nodes.
  /// @note The node is not printed in XML format.
  void print ( Uint nesting = 0 ) const;

  /// Deep copies this node into another with all the memory allocated in the second.
  /// @param out The destination node.
  void deep_copy ( XmlNode& out ) const;

private: // helper functions

  /// Helper function that copy the names and values to the memory pool of the out node
  /// @pre Assumes that the nodes have been cloned before.
  /// @pre Assumes that the destination node is valid.
  /// @param in The source node.
  /// @param out The destination node.
  void deep_copy_names_values ( const XmlNode& in, XmlNode& out ) const;

}; // XmlNode

////////////////////////////////////////////////////////////////////////////

} // XML
} // common
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_XML_XmlNode_hpp
