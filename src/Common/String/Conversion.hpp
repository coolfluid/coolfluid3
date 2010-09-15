// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_String_Conversion_hpp
#define CF_Common_String_Conversion_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"
#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace String {
  
////////////////////////////////////////////////////////////////////////////////

/// Conversions from and to std::string

  /// Converts to std::string
  /// Don't use this to convert to a char, use c_str for that.
  /// Typical use is to convert to numbers.
  /// @param str string to convert from
  /// @return converter type
  template < typename T>
  Common_API std::string to_str (const T & v);


  /// Converts from std::string
  /// Don't use this to convert to a char, use c_str for that.
  /// Typical use is to convert to numbers.
  /// @param str string to convert from
  /// @return converter type
  template < typename T>
  Common_API T from_str (const std::string& str);

////////////////////////////////////////////////////////////////////////////////

} // namespace String
} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_String_Conversion_hpp
