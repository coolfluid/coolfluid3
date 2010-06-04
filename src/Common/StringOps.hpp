#ifndef CF_StringOps_hpp
#define CF_StringOps_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "Common/CF.hpp"
#include "Common/NonInstantiable.hpp"
#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// Operations on std::string
/// @author Tiago Quintino
class Common_API StringOps : public Common::NonInstantiable<StringOps> {

public:

  /// Converts to std::string
  /// Don't use this to convert to a char, use c_str for that.
  /// Typical use is to convert to numbers.
  /// @param str string to convert from
  /// @return converter type
  template <class T>
  static std::string to_str (const T & v)
  {
    std::ostringstream oss;
    oss << v;
    return oss.str();
  }

  /// Converts from std::string
  /// Don't use this to convert to a char, use c_str for that.
  /// Typical use is to convert to numbers.
  /// @param str string to convert from
  /// @return converter type
  template <class T>
  static T from_str (const std::string& str)
  {
    T v;
    if (str.length() > 0)
    {
      std::istringstream iss(str.c_str());
      iss >> v;
    }
    else
    {
      // pretty much everything has an empty constuctor
      v = T();
    }
    return v;
  }

}; // class StringOps

////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_StringOps_hpp
