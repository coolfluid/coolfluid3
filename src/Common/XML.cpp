/// @todo tmp header boost/filesystem/path, should go!
#include <boost/filesystem/path.hpp>

#include "Common/BasicExceptions.hpp"
#include "Common/URI.hpp"
#include "Common/XML.hpp"
#include "Common/String/Conversion.hpp"

namespace CF {
namespace Common {
  
  using namespace String;

////////////////////////////////////////////////////////////////////////////////

template<>
Common_API const char * XmlTag<bool>::type() { return "bool"; }

template<>
Common_API const char * XmlTag<int>::type() { return "integer"; };

template<>
Common_API const char * XmlTag<CF::Uint>::type() { return "unsigned"; }

template<>
Common_API const char * XmlTag<CF::Real>::type() { return "real"; }

template<>
Common_API const char * XmlTag<std::string>::type() { return "string"; }

template<>
Common_API const char * XmlTag<boost::filesystem::path>::type() { return "file"; }

template<>
Common_API const char * XmlTag<URI>::type() { return "uri"; }

  
////////////////////////////////////////////////////////////////////////////////

template <>
Common_API std::string from_value<bool> (const bool& val)
{
  return to_str(val);
}

template <>
Common_API std::string from_value<int> (const int& val)
{
  return to_str(val);
}

template <>
Common_API std::string from_value<Uint> (const Uint& val)
{
  return to_str(val);
}

template <>
Common_API std::string from_value<Real> (const Real& val)
{
  return to_str(val);
}

template <>
Common_API std::string from_value<std::string> (const std::string& val)
{
  return val;
}

template <>
Common_API std::string from_value<URI> (const URI& val)
{
  return to_str(val);
}

////////////////////////////////////////////////////////////////////////////////

template <>
Common_API void to_value<bool> (XmlBase& node, bool& val)
{
  val = from_str<bool>(node.value());
}

template <>
Common_API void to_value<int> (XmlBase& node, int& val)
{
  val = from_str<int>(node.value());
}

template <>
Common_API void to_value<Uint> (XmlBase& node, Uint& val)
{
  val = from_str<Uint>(node.value());
}

template <>
Common_API void to_value<Real> (XmlBase& node, Real& val)
{
  val = from_str<Real>(node.value());
}

template <>
Common_API void to_value<std::string> (XmlBase& node, std::string& val)
{
  val = node.value();
}

template <>
Common_API void to_value<URI> (XmlBase& node, URI& val)
{
  val = from_str<URI>(node.value());
}

////////////////////////////////////////////////////////////////////////////////

template <>
Common_API bool to_value<bool> (XmlBase& node)
{
  return from_str<bool>(node.value());
}

template <>
Common_API int to_value<int> (XmlBase& node)
{
  return from_str<int>(node.value());
}

template <>
Common_API Uint to_value<Uint> (XmlBase& node)
{
  return from_str<Uint>(node.value());
}

template <>
Common_API Real to_value<Real> (XmlBase& node)
{
  return from_str<Real>(node.value());
}

template <>
Common_API std::string to_value<std::string> (XmlBase& node)
{
  return node.value();
}

template <>
Common_API URI to_value<URI> (XmlBase& node)
{
  return from_str<URI>(node.value());
}
  
////////////////////////////////////////////////////////////////////////////////
  
  
/// @todo Temporary, should GO!!!
template <>
Common_API boost::filesystem::path to_value<boost::filesystem::path> (XmlBase& node)
{
  return boost::filesystem::path(node.value());
}
  
template <>
Common_API void to_value<boost::filesystem::path> (XmlBase& node, boost::filesystem::path& val)
{
  val = boost::filesystem::path(node.value());
}
  
template <>
Common_API std::string from_value<boost::filesystem::path> (const boost::filesystem::path& val)
{
  return val.string();
}

} // Common
} // CF
