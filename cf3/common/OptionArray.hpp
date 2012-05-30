// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_OptionArray_hpp
#define cf3_common_OptionArray_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Option.hpp"
#include "common/StringConversion.hpp"
#include "common/TypeInfo.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

/// Class defines an array of options of the same type to be used by the ConfigObject class
/// This class supports the following types:
///   - bool
///   - int
///   - std::string
///   - boost::filesystem::path
///   - cf3::Uint
///   - cf3::Real
///   - cf3::common::URI
/// @author Tiago Quintino
template < typename TYPE >
class Common_API OptionArray : public Option  {

public:
  typedef std::vector<TYPE> value_type;
  typedef TYPE ElementT;

  OptionArray( const std::string& name, const value_type& def);

  virtual ~OptionArray() {}

  /// @name VIRTUAL FUNCTIONS
  //@{

  /// Returns a C-strng representation of element type
  virtual std::string element_type() const { return class_name<TYPE>(); }

  virtual std::string value_str() const
  {
    const value_type val = value<value_type>();

    // build the value string
    return option_vector_to_str(val, separator());
  }

  virtual std::string restricted_list_str() const;
  virtual void set_restricted_list_str(const std::vector< std::string >& list);

  //@} END VIRTUAL FUNCTIONS

private:
  virtual void copy_to_linked_params(std::vector< boost::any >& linked_params);
  virtual boost::any extract_configured_value(XML::XmlNode& node);
  virtual void change_value_impl(const boost::any& value);
}; // class OptionArray

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_OptionArray_hpp
