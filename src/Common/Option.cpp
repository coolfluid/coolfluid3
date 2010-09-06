#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/Option.hpp"
#include "Common/CPath.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

  Option::Option ( const std::string& name,
                   const std::string& type,
                   const std::string& desc,
                   boost::any def) :
  m_value(def),
  m_default(def),
  m_name(name),
  m_type(type),
  m_description(desc)
  {
  }

  Option::~Option()
  {
  }

  void Option::configure_option ( XmlNode& node )
  {
    this->change_value(node); // update the value

    // call all process functors
    BOOST_FOREACH( Option::Processor_t& process, m_processors )
        process();
  }

  template<>
  Common_API const char * Option::type_to_str<bool>() const { return "bool"; }

  template<>
  Common_API const char * Option::type_to_str<int>() const { return "integer"; };

  template<>
  Common_API const char * Option::type_to_str<CF::Uint>() const { return "unsigned"; }

  template<>
  Common_API const char * Option::type_to_str<CF::Real>() const { return "real"; }

  template<>
  Common_API const char * Option::type_to_str<std::string>() const { return "string"; }

  template<>
  Common_API const char * Option::type_to_str<boost::filesystem::path>() const { return "file"; }

  template<>
  Common_API const char * Option::type_to_str<CPath>() const { return "component"; }

  void Option::mark_basic()
  {
    if(!has_tag("basic"))
      add_tag("basic");
  }

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
