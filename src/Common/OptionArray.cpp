#include "Common/OptionArray.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  OptionArray::OptionArray(const std::string& name, const std::string& type,
                           const std::string& desc, const boost::any def) :
      Option(name,type, desc, def)
  {

  }


////////////////////////////////////////////////////////////////////////////////
  template<>
  const char * OptionArrayT<bool>::elem_type() const { return "bool"; }

  template<>
  const char * OptionArrayT<int>::elem_type() const { return "integer"; };

  template<>
  const char * OptionArrayT<CF::Uint>::elem_type() const { return "unsigned"; }

  template<>
  const char * OptionArrayT<CF::Real>::elem_type() const { return "real"; }

  template<>
  const char * OptionArrayT<std::string>::elem_type() const { return "string"; }

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
