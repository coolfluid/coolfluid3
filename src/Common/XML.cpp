#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/compare.hpp>

#include "Common/BasicExceptions.hpp"
#include "Common/XML.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  template<>
  const char * XmlTag<bool>::str() { return "bool"; }

  template<>
  const char * XmlTag<int>::str() { return "integer"; };

  template<>
  const char * XmlTag<CF::Uint>::str() { return "integer"; }

  template<>
  const char * XmlTag<CF::Real>::str() { return "real"; }

  template<>
  const char * XmlTag<std::string>::str() { return "string"; }

////////////////////////////////////////////////////////////////////////////////

template <>
    void xmlstr_to_value ( XmlNode& node, bool& val )
{
  bool error = true;
  std::string vt ( node.value() );

  boost::algorithm::is_equal test_equal;

  if ( test_equal(vt,"true") ||
       test_equal(vt,"on")   ||
       test_equal(vt,"1")     )
  {
    val   = true;
    error = false;
  }

  if ( !error && (
       test_equal(vt,"false") ||
       test_equal(vt,"off")   ||
       test_equal(vt,"0")     ))
  {
      val   = false;
      error = false;
  }

  if (error)
    throw ParsingFailed (FromHere(), "Incorrect option conversion to bool of string [" + vt + "]" );
}

template <>
    void xmlstr_to_value ( XmlNode& node, int& val )
{
   val = boost::lexical_cast<int> ( node.value() );
}

template <>
    void xmlstr_to_value ( XmlNode& node, CF::Uint& val )
{
  val = boost::lexical_cast<CF::Uint> ( node.value() );
}

template <>
    void xmlstr_to_value ( XmlNode& node, CF::Real& val )
{
  val = boost::lexical_cast<CF::Real> ( node.value() );
}

template <>
    void xmlstr_to_value ( XmlNode& node, std::string& val )
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
  return boost::lexical_cast<std::string> ( val );
}

template <>
    std::string value_to_xmlstr<std::string> ( const std::string& val )
{
   return val;
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
