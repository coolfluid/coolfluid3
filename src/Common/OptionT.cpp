#include "Common/OptionT.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

template<>
const char * OptionT<bool>::tag() const { return "bool"; }

template<>
const char * OptionT<int>::tag() const { return "integer"; };

template<>
const char * OptionT<CF::Uint>::tag() const { return "integer"; }

template<>
const char * OptionT<CF::Real>::tag() const { return "real"; }

template<>
const char * OptionT<std::string>::tag() const { return "string"; }

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
