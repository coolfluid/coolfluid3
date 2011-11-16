// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_OptionArray_hpp
#define cf3_common_OptionArray_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Option.hpp"
#include "common/XML/Protocol.hpp"

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
  virtual std::string element_type() const { return std::string(XML::Protocol::Tags::type<ElementT>()); }

  /// @returns the xml tag for this option
  virtual const char * tag() const { return XML::Protocol::Tags::node_array(); }
  
  virtual std::string value_str() const
  {
    return to_str( value<TYPE>() );
  }

  //@} END VIRTUAL FUNCTIONS

private:
  virtual void copy_to_linked_params(std::vector< boost::any >& linked_params);
  virtual boost::any extract_configured_value(XML::XmlNode& node);
}; // class OptionArray

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_OptionArray_hpp
