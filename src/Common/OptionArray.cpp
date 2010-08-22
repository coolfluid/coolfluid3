#include "Common/OptionArray.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////
  template<>
  const char * OptionArray<bool>::type_tag() const { return "bool"; }

  template<>
  const char * OptionArray<int>::type_tag() const { return "integer"; };

  template<>
  const char * OptionArray<CF::Uint>::type_tag() const { return "unsigned"; }

  template<>
  const char * OptionArray<CF::Real>::type_tag() const { return "real"; }

  template<>
  const char * OptionArray<std::string>::type_tag() const { return "string"; }

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
