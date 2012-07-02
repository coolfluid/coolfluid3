// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_OptionT_hpp
#define cf3_common_OptionT_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Option.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

/// Class defines one option to be used by the ConfigObject class
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
    class Common_API OptionT : public Option  {

public:

  typedef TYPE value_type;

  OptionT ( const std::string& name, value_type def);

  /// @name VIRTUAL FUNCTIONS
  //@{

  /// @returns the value as a std::string
  virtual std::string value_str () const;

  virtual std::string restricted_list_str() const;
  virtual void set_restricted_list_str(const std::vector< std::string >& list);

  //@} END VIRTUAL FUNCTIONS

private: // functions

  /// copy the configured update value to all linked parameters
  virtual void copy_to_linked_params (std::vector< boost::any >& linked_params );

  virtual boost::any extract_configured_value(XML::XmlNode& node);

  virtual void change_value_impl(const boost::any& value);

}; // class OptionT

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_OptionT_hpp
