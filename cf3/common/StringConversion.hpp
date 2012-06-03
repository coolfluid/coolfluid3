// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_StringConversion_hpp
#define cf3_common_StringConversion_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CommonAPI.hpp"
#include "Handle.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

/// @file StringConversion.hpp
/// @brief Conversions from and to std::string

  /// @brief Converts to std::string
  ///
  /// Don't use this to convert to a char, use c_str for that.
  /// Typical use is to convert numbers to a string.
  /// @param v value to convert to string
  /// @return converter type
  template < typename T>
  Common_API std::string to_str (const T & v);

  template<typename ComponentT>
  Common_API std::string to_str(const Handle<ComponentT> & v)
  {
    return to_str(v->uri());
  }


  /// @brief Converts from std::string
  ///
  /// Typical use is to convert a string to numbers.
  /// @param str string to convert from
  /// @return string
  template < typename T>
  Common_API T from_str (const std::string& str);

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_StringConversion_hpp
