// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_XmlSignature_hpp
#define CF_Common_XmlSignature_hpp

////////////////////////////////////////////////////////////////////////////

#include "Common/PropertyList.hpp"
#include "Common/XmlHelpers.hpp"
#include "Common/XML.hpp"

#include "Common/CommonAPI.hpp"

#include "Common/Log.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  //////////////////////////////////////////////////////////////////////////

  /// @brief Describes how a map should look like to be accepted by
  /// a signal/configuration.

  /// This class describes the "minimal" requirements. Those ones have
  /// to be present in the map. Additional values are accepted.
  /// @author Quentin Gasper
  class Common_API XmlSignature
  {

  public:

    /// @brief Creates a XmlSignature
    /// @param node The map with required values for this signature.
    /// @return Returns the created signature.
    static XmlSignature make_signature(const XmlNode & node);

    /// @brief Constructor
    XmlSignature();

    /// @brief Constructor with required values
    /// @param node The node with the required values for this signature.
    XmlSignature(const XmlNode & node);

    /// @brief Destructor
    /// Frees all allocated nodes.
    ~XmlSignature();

    /// @brief Appends the list of the value to the provided node.
    /// @param node The node to which data will be appended to. Should be
    /// a map.
    void put_signature(XmlNode & node) const;

    /// @brief Checks whether the provided node contains at least the
    /// expected values.
    /// @param node The node to check. Should be a map.
    /// @return Returns @c true if the node contains at least all the
    /// expected data; otherwise, returns @c false.
    bool validate(const XmlNode & node) const;

    /// @brief Gives the parent.
    /// @return Returns a reference to the parent.
    XmlSignature & back();

    /// @brief Adds a map.
    /// @param name map name.
    /// @param desc map description
    /// @return Returns the newly created map.
    XmlSignature & insert_map(const std::string & name, const std::string & desc);

    /// @brief Adds a data description of type T.
    /// T should be a type supported by the Xml protocol.
    /// @param name The description name. Can not be empty.
    /// @param desc A description string. May be empty.
    /// @param default_value Default value.
    /// @param restr_list List of restricted values.
    /// @return Returns a reference to this object.
    template<typename T>
    XmlSignature & insert(const std::string & name,
                          const std::string & desc = std::string(),
                          const T & default_value = T(),
                          const std::vector<T> & restr_list = std::vector<T>());

    /// @brief Adds a data description for an array with items of type T.
    /// T should be a type supported by the Xml protocol.
    /// @param name The description name. Can not be empty.
    /// @param desc A description string. May be empty.
    /// @param default_value Default value.
    /// @param restr_list List of restricted values.
    /// @return Returns a reference to this object.
    template<typename T>
    XmlSignature & insert_array(const std::string & name,
                                const std::string & desc = std::string(),
                                const std::vector<T> & default_value = std::vector<T>(),
                                const std::vector<T> & restr_list = std::vector<T>());

  private:

    /// @brief The signature parent. May be null.
    XmlSignature * m_parent;

    boost::shared_ptr<XmlDoc> m_xmldoc;

    /// @brief The value storage.
    /// Points to a map node.
    XmlNode * m_data;

    /// @brief The map children.
    /// The key in is the map key (name). The value is the object
    /// that manages the map.
    std::map<std::string, XmlSignature *> m_maps;

    /// @brief Private constructor used build child XmlSignatures
    /// @param node A pointer to the map the object has to manage
    /// @param parent The parent object.
    XmlSignature(XmlNode * node, XmlSignature * parent);

    /// @brief Makes a deep copy of @c in child nodes to @c out.

    /// Existing child nodes under @c out are not deleted and its name is not
    /// modified.
    /// @param in Node to copy.
    /// @param out Node where the copy has to be appended to.
    void copy_node(const XmlNode & in, XmlNode & out) const;

  }; // class XmlSignature

  //////////////////////////////////////////////////////////////////////////

  template<typename TYPE>
  XmlSignature & XmlSignature::insert(const std::string & name,
                                      const std::string & desc,
                                      const TYPE & default_value,
                                      const std::vector<TYPE> & restr_list)
  {
    cf_assert( !name.empty() );

    XmlNode * node;

    if( XmlParams::check_key_in(*m_data, name) )
    {
      throw ValueExists(FromHere(), "A value with name [" + name + "] already exists.");
    }

    node = XmlParams::add_value_to(*m_data, name, default_value);

    XmlOps::add_attribute_to(*node, XmlParams::tag_attr_descr(), desc);

    if( !restr_list.empty() )
      XmlParams::add_array_to(*node, XmlParams::tag_attr_restricted_values(),
                              restr_list);

    return *this;
  }

  //////////////////////////////////////////////////////////////////////////

  template<typename TYPE>
  XmlSignature & XmlSignature::insert_array(const std::string & name,
                                            const std::string & desc,
                                            const std::vector<TYPE> & default_value,
                                            const std::vector<TYPE> & restr_list)
  {
    cf_assert( !name.empty() );

    XmlNode * node;

    if( XmlParams::check_key_in(*m_data, name) )
    {
      throw ValueExists(FromHere(), "An array with name [" + name + "] already exists.");
    }

    node = XmlParams::add_array_to(*m_data, name, default_value);

    XmlOps::add_attribute_to(*node, XmlParams::tag_attr_descr(), desc);

    if( !restr_list.empty() )
      XmlParams::add_array_to(*node, XmlParams::tag_attr_restricted_values(),
                              restr_list);

    return *this;
  }

  //////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_XmlSignature_hpp
