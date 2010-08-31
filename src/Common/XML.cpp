#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/compare.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/BasicExceptions.hpp"
#include "Common/XML.hpp"

namespace CF {
namespace Common {

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

////////////////////////////////////////////////////////////////////////////////

template <>
    Common_API void xmlstr_to_value ( XmlBase& node, bool& val )
{
  bool match = false;
  std::string vt ( node.value() );

  boost::algorithm::is_equal test_equal;

  if ( test_equal(vt,"true") ||
       test_equal(vt,"on")   ||
       test_equal(vt,"1")     )
  {
    val   = true;
    match = true;
  }

  if ( !match && (
       test_equal(vt,"false") ||
       test_equal(vt,"off")   ||
       test_equal(vt,"0")     ))
  {
      val   = false;
      match = true;
  }

  if (!match)
    throw ParsingFailed (FromHere(), "Incorrect option conversion to bool of string [" + vt + "]" );
}

template <>
    Common_API void xmlstr_to_value ( XmlBase& node, int& val )
{
   val = boost::lexical_cast<int> ( node.value() );
}

template <>
    Common_API void xmlstr_to_value ( XmlBase& node, CF::Uint& val )
{
  val = boost::lexical_cast<CF::Uint> ( node.value() );
}

template <>
    Common_API void xmlstr_to_value ( XmlBase& node, CF::Real& val )
{
  val = boost::lexical_cast<CF::Real> ( node.value() );
}

template <>
    Common_API void xmlstr_to_value ( XmlBase& node, std::string& val )
{
   val = node.value();
}

template <>
    Common_API void xmlstr_to_value ( XmlBase& node, boost::filesystem::path& val )
{
   val = node.value();
}

////////////////////////////////////////////////////////////////////////////////

template <>
    Common_API std::string value_to_xmlstr<bool> ( const bool& val )
{
  return val ? "true" : "false";
}

template <>
    Common_API std::string value_to_xmlstr<int> ( const int& val )
{
  return boost::lexical_cast<std::string> ( val );
}

template <>
    Common_API std::string value_to_xmlstr<CF::Uint> ( const CF::Uint& val )
{
  return boost::lexical_cast<std::string> ( val );
}

template <>
    Common_API std::string value_to_xmlstr<CF::Real> ( const CF::Real& val )
{
  std::stringstream ss;
  ss << val;
  return ss.str();
}

template <>
    Common_API std::string value_to_xmlstr<std::string> ( const std::string& val )
{
   return val;
}

template <>
    Common_API std::string value_to_xmlstr<boost::filesystem::path> ( const boost::filesystem::path & val )
{
   return val.string();
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
