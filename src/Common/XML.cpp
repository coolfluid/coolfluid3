#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/compare.hpp>

#include "Common/BasicExceptions.hpp"
#include "Common/XML.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  template<>
  const char * XmlTag<bool>::type() { return "bool"; }

  template<>
  const char * XmlTag<int>::type() { return "integer"; };

  template<>
  const char * XmlTag<CF::Uint>::type() { return "unsigned"; }

  template<>
  const char * XmlTag<CF::Real>::type() { return "real"; }

  template<>
  const char * XmlTag<std::string>::type() { return "string"; }

////////////////////////////////////////////////////////////////////////////////

template <>
    void xmlstr_to_value ( XmlBase& node, bool& val )
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
    void xmlstr_to_value ( XmlBase& node, int& val )
{
   val = boost::lexical_cast<int> ( node.value() );
}

template <>
    void xmlstr_to_value ( XmlBase& node, CF::Uint& val )
{
  val = boost::lexical_cast<CF::Uint> ( node.value() );
}

template <>
    void xmlstr_to_value ( XmlBase& node, CF::Real& val )
{
  val = boost::lexical_cast<CF::Real> ( node.value() );
}

template <>
    void xmlstr_to_value ( XmlBase& node, std::string& val )
{
   val = node.value();
}

////////////////////////////////////////////////////////////////////////////////

template <>
    std::string value_to_xmlstr<bool> ( const bool& val )
{
  return val ? "true" : "false";
}

template <>
    std::string value_to_xmlstr<int> ( const int& val )
{
  return boost::lexical_cast<std::string> ( val );
}

template <>
    std::string value_to_xmlstr<CF::Uint> ( const CF::Uint& val )
{
  return boost::lexical_cast<std::string> ( val );
}

template <>
    std::string value_to_xmlstr<CF::Real> ( const CF::Real& val )
{
  std::stringstream ss;
  ss << val;
  return ss.str();
}

template <>
    std::string value_to_xmlstr<std::string> ( const std::string& val )
{
   return val;
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
