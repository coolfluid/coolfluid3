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
