// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_XML_Map_hpp
#define cf3_common_XML_Map_hpp

/////////////////////////////////////////////////////////////////////////////////

#include "common/CF.hpp"
#include "common/XML/XmlNode.hpp"

namespace cf3 {
namespace common {
namespace XML {

/////////////////////////////////////////////////////////////////////////////////

  /// Manages maps in a XML tree.
  /// A map is an associative container of values: it contains values identified
  /// by a unique name (the "key"). Each value can contain another
  /// map. There are two kind of values: single values and array values.
  /// Single values contain only one value of a certain type and array ones
  /// may contain several values (each of the same type). @n

  /// A value has an assigned type (one of those supported by @c #from_value() )
  /// which cannot be changed (from example, a @c bool value cannot be changed
  /// to @c int). An existing value can be modified using @c #set_value() or
  /// @c #set_array(). If it does not exist, these function automatically add it.@n

  /// A value can have a restricted list of values. This is an array that
  /// defines the acceptable values. Setting a restricted to value is the
  /// same as setting a array to it.

  /// @author Quentin Gasper.
  class Common_API Map
  {
  public: // methods

    /// Constructor.
    /// @param node The node to manage. Must be valid.
    Map ( XmlNode node = XmlNode() );

    /// Adds or modifies a value.
    /// The TYPE must be one of those supported by @c #from_value().
    /// If no value with the provided key exists, a single value is created.
    /// If it already exists @b and is a single value, it is modified.
    /// An array value cannot be modified as a single one. @n

    /// @param value_key The value key (name). Cannot be empty.
    /// @param type_name The string description of the type
    /// @param value_str The value, as string
    /// @param descr Description

    /// @throw BadValue If the value key is empty.
    /// @throw XmlError if
    ///        @li the value exists and is an array
    ///        @li the value exists and its type does not match to TYPE
    ///        @li the value exists but does not have a type
    XmlNode set_value ( const std::string& value_key, const std::string type_name, const std::string& value_str,
                        const std::string& descr = std::string());

    /// Adds or modifies an array value.
    /// The TYPE must be one of those supported by @c #from_value().
    /// If no value with the provided key exists, a single value is created.
    /// If it already exists @b and is a single value, it is modified.
    /// An array value cannot be modified as a single one. @n

    /// @param value_key The value key (name). Cannot be empty.
    /// @param type_name The string description of the type of a single element
    /// @param value_str The value, as string, for the entire array
    /// @param descr Description

    /// @throw BadValue If the value key is empty.
    XmlNode set_array ( const std::string& value_key, const std::string element_type_name, const std::string& value_str, const std::string& delimiter,
                        const std::string& descr = std::string());

    /// Searches for a value in this map.

    /// @param value_key The key (name) of the wanted value. May be empty.
    /// @param value_type The value type (single value or array). Three values
    /// are accepted : @c Protocol::Tags::node_value() (for a value),
    /// @c Protocol::Tags::node_array() (for an array) and an empty string (any
    /// type).

    /// @return Returns a pointer to the first value found with the key.
    /// If the key is empty, returns the first value found in the map. Otherwise,
    /// returns a null pointer. If @c value_type is not valid, a null pointer
    /// is returned as well.
    XmlNode find_value ( const std::string & value_key = std::string(),
                         const char * value_type = nullptr ) const;

    /// Checks whether an entry exists or not.

    /// An entry can be either a single value, or a map.
    /// @param entry_key The entry name
    /// @return Returns @c true if the entry was found.
    bool check_entry ( const std::string & entry_key ) const;

    /// Checks whether the provided node represents a single value.
    /// @note This function does not check the validity of the XML
    /// representation (i.e. it doesn't check that the type exists or is
    /// valid,...).
    /// @param node The node to check.
    /// @return Returns @c true if the node represents a single value.
    /// Otherwise, returns @c false.
    static bool is_single_value ( const XmlNode& node );

    /// Checks whether the provided node represents an array value.
    /// @note This function does not check the validity of the XML
    /// representation (i.e. it doesn't check that the type exists or is
    /// valid,...).
    /// @param node The node to check.
    /// @return Returns @c true if the node represents a array value.
    /// Otherwise, returns @c false.
    static bool is_array_value ( const XmlNode& node );

    /// Gives the type name of a provided value node.
    /// @param node The node to process. Must be valid.
    /// @return Returns a C-string with the type name.
    /// @note The function does not check whether the
    /// @throw XmlError If
    ///        @li the node is neither a single nor an array value
    ///        @li no type was found inside the node
    static const char * get_value_type ( const XmlNode& node );

    /// Searches a value in the provided map and converts it to TYPE.
    /// @param map The map.
    /// @param val_key The value key (name). Cannot be empty.
    /// @return Returns the converted value.
    /// @throw BadValue If the value key is empty.
    /// @throw XmlError If no value with that key and value was found.
    template<typename TYPE>
    TYPE get_value ( const std::string& val_key ) const;

    /// Searches an array in the provided map and converts it to std::vector<TYPE>.
    /// @note This function can be used to get a restricted list of a value.
    /// @param map The map under which the array has to searched for.
    /// @param val_key The array key.
    /// @return Returns a vector containing the array values. The vector
    /// might be empty as the array may have no value.
    /// @throw BadValue If the value key is empty.
    /// @throw XmlError If no value with that key and value was found.
    template<typename TYPE>
    std::vector<TYPE> get_array ( const std::string& val_key ) const;

    /// Converts an array XML node to a vector of TYPE
    /// @param array_node The array node to convert. Must be valid.
    /// @param delim A pointer a string where the delimiter will be stored. Can be NULL.
    /// @throw XmlError if no delimiter is found.
    /// @throw ParsingFailed if no size is found or if the found size does not
    /// match with the number of found elements.
    template<typename TYPE>
    std::vector<TYPE> array_to_vector ( const XmlNode & array_node, std::string * delim = nullptr ) const;

    /// Splits a string and casts each resulting part to TYPE.
    /// This function can whether the number of elements to read is known or
    /// or not. If the size is known the provided vector is cleared and
    /// initialized to contain this number of items. This way is more efficient,
    /// of course, because there is no reallocation needed.
    /// If the size if not defined, elements are appended to the vector by
    /// using @c push_back().
    /// @param str The string to split.
    /// @param delimiter The string that delimitates the parts. It will not
    /// appear in the @c result.
    /// @param result The vector where cast parts are stored.
    /// @param size If greater or equal to 0, gives the number of elements to
    /// read. Otherwise, the vector is reallocated as the elements are added.
    /// @throw CastingFailed If an item cannot be cast to TYPE.
    template <typename TYPE>
    static void split_string ( const std::string & str, const std::string & delimiter,
                               std::vector<TYPE> & result, int size = -1 );

  public: // data

    /// The managed node.
    XmlNode content;

  private: // helper functions

    /// Checks whether the provided has the type TYPE.
    /// This function can be called for both single and array values.
    /// @param node The node to check.
    /// @return Returns @c true if the value has the correct type. Otherwise,
    /// or if the node is not valid or is neither a signle value nor an
    /// array value, returns @c false.
    template <typename TYPE>
    static bool value_has_ptr ( const XmlNode& node );

  }; // Map

/////////////////////////////////////////////////////////////////////////////////

} // XML
} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_XML_Map_hpp
