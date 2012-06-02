// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_OptionArray_hpp
#define cf3_common_OptionArray_hpp

////////////////////////////////////////////////////////////////////////////////

#include "boost/algorithm/string.hpp"

#include "common/Option.hpp"
#include "common/OptionArrayDetail.hpp"
#include "common/StringConversion.hpp"
#include "common/TypeInfo.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

/// Class defines an array of options of the same type
/// @author Tiago Quintino
/// @author Bart Janssens
template < typename TYPE >
class Common_API OptionArray : public Option  {

public:
  typedef std::vector<TYPE> value_type;
  typedef TYPE ElementT;

  OptionArray( const std::string& name, const value_type& def) : Option(name, def)
  {
  }

  virtual ~OptionArray() {}

  /// @name VIRTUAL FUNCTIONS
  //@{

  /// Returns a C-strng representation of element type
  virtual std::string element_type() const { return class_name<TYPE>(); }

  virtual std::string value_str() const
  {
    const value_type val = Option::template value<value_type>();

    // build the value string
    return option_vector_to_str(val, separator());
  }

  virtual boost::any value() const
  {
    return detail::Value<TYPE>()(m_value);
  }

  /// Redirect form the base class due to conflict with the non-template
  template<typename OutputT>
  OutputT value() const
  {
    return Option::template value<OutputT>();
  }

  virtual std::string restricted_list_str() const
  {
    std::vector<TYPE> restr_list_vec;
    BOOST_FOREACH(const boost::any& restr_item, restricted_list())
    {
      restr_list_vec.push_back(boost::any_cast<TYPE>(restr_item));
    }
    return option_vector_to_str(restr_list_vec, separator());
  }

  virtual void set_restricted_list_str(const std::vector< std::string >& list)
  {
    BOOST_FOREACH(const std::string& item, list)
    {
      restricted_list().push_back(detail::FromString<TYPE>()(item));
    }
  }

  //@} END VIRTUAL FUNCTIONS

private:
  virtual void copy_to_linked_params(std::vector< boost::any >& linked_params)
  {
    std::vector<TYPE> val = Option::template value< std::vector<TYPE> >();
    BOOST_FOREACH ( boost::any& v, linked_params )
    {
      std::vector<TYPE>* cv = boost::any_cast<std::vector<TYPE>*>(v);
      *cv = val;
    }
  }

  virtual boost::any extract_configured_value(XML::XmlNode& node)
  {
    rapidxml::xml_attribute<>* attr = node.content->first_attribute( "type" );

    if ( !attr )
      throw ParsingFailed (FromHere(), "OptionArray does not have \'type\' attribute" );

    const std::string node_type(attr->value());
    if (node_type != element_type() && !(node_type == "integer" && (element_type() == "real") || (element_type() == "unsigned") ) && !boost::starts_with(node_type, "handle["))
      throw ParsingFailed (FromHere(), "OptionArray expected \'type\' attribute \'"
      +  std::string(attr->value())
      + "\' but got \'"
      +  std::string(element_type()) + "\'"  );

    return detail::ArrayToVector<TYPE>()(node);
  }

  virtual void change_value_impl(const boost::any& value)
  {
    detail::ChangeArrayValue<TYPE>()(m_value, value);
  }
}; // class OptionArray

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_OptionArray_hpp
