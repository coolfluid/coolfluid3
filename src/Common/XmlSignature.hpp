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

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  //////////////////////////////////////////////////////////////////////////

  /// @brief Describes how a valuemap should look like to be accepted by
  /// a signal/configuration.

  /// This class describes the "minimal" requirements. Those ones have
  /// to be present in the valuemap. Additional values are accepted.
  /// @author Quentin Gasper
  class Common_API XmlSignature
  {

  public:

    /// @brief Creates a XmlSignature
    /// @param node The valuemap with required values for this signature.
    /// @return Returns the created signature.
    static XmlSignature make_signature(const XmlNode & node);

    /// @brief Constructor
    XmlSignature();

    /// @brief Constructor with required values
    /// @param node The node with the required values for this signature.
    XmlSignature(const XmlNode & node);

    /// @brief Destructor
    /// Frees all allocated node if the signature has node parent.
    ~XmlSignature();

    /// @brief Appends the list of the value to the provided node.
    /// @param node The node to which data will be appended to. Should be
    /// a valuemap.
    void put_signature(XmlNode & node) const;

    /// @brief Checks whether the provided node contains at least the
    /// expected values.
    /// @param node The node to check. Should be a valuemap.
    /// @return Returns @c true if the node contains at least all the
    /// expected data; otherwise, returns @c false.
    bool validate(const XmlNode & node) const;

    /// @brief Gives the parent.
    /// @return Returns a reference to the parent.
    XmlSignature & back();

    /// @brief Adds a valuemap.
    /// @param name Valuemap name.
    /// @param desc Valuemap description
    /// @return Returns the newly created valuemap.
    XmlSignature & insert_valuemap(const std::string & name, const std::string & desc);

    /// @brief Adds a data description of type T.
    /// Type should be a subclass of @c #Option class.
    /// @param name The description name. Can not be empty.
    /// @param desc A description string. May be empty.
    /// @return Returns a reference to this object.
    template<typename T>
    XmlSignature & insert(const std::string & name,
                          const std::string & desc = std::string(),
                          bool is_array = false);

  private:

    /// @brief The signature parent. May be null.
    XmlSignature * m_parent;

    boost::shared_ptr<XmlDoc> m_xmldoc;

    /// @brief The value storage.
    XmlNode * m_data;

    /// @brief The valuemap children.
    /// The key in is the valuemap key (name). The value is the object
    /// that manages the valuemap.
    std::map<std::string, XmlSignature *> m_valuemaps;

    /// @brief Private constructor used build child XmlSignatures
    /// @param node A pointer to the valuemap the object has to manage
    /// @param parent The parent object.
    XmlSignature(XmlNode * node, XmlSignature * parent);

    void copy_node(const XmlNode & in, XmlNode & out) const;

  }; // class XmlSignature

  //////////////////////////////////////////////////////////////////////////

  template<typename TYPE>
  XmlSignature & XmlSignature::insert(const std::string & name,
                                      const std::string & desc,
                                      bool is_array)
  {
    cf_assert( !name.empty() );

    XmlNode * node;

    if( XmlParams::check_key_in(*m_data, name) )
    {
      throw ValueExists(FromHere(), "A value with name [" + name + "] already exists.");
    }

    if( !is_array )
      node = XmlParams::add_value_to(*m_data, name, TYPE());
    else
      node = XmlParams::add_array_to(*m_data, name, std::vector<TYPE>());

    XmlOps::add_attribute_to(*node, XmlParams::tag_attr_descr(), desc);

    return *this;
  }

  //////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_XmlSignature_hpp
