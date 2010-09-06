#include <boost/filesystem/path.hpp>

#include "Common/OptionArray.hpp"
#include "Common/CPath.hpp"

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
  Common_API const char * OptionArrayT<bool>::elem_type() const { return "bool"; }

  template<>
  Common_API const char * OptionArrayT<int>::elem_type() const { return "integer"; };

  template<>
  Common_API const char * OptionArrayT<CF::Uint>::elem_type() const { return "unsigned"; }

  template<>
  Common_API const char * OptionArrayT<CF::Real>::elem_type() const { return "real"; }

  template<>
  Common_API const char * OptionArrayT<std::string>::elem_type() const { return "string"; }

  template<>
  Common_API const char * OptionArrayT<boost::filesystem::path>::elem_type() const { return "file"; }

  template<>
  Common_API const char * OptionArrayT<CPath>::elem_type() const { return "component"; }

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
