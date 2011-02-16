// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "rapidxml/rapidxml.hpp"

#include "Common/CommonAPI.hpp"
#include "Common/URI.hpp"
#include "Common/String/Conversion.hpp"

#include "Common/XML/CastingFunctions.hpp"

using namespace CF::Common::String;

namespace CF {
namespace Common {
namespace XML {

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
  Common_API void to_value<bool> (XmlNode& node, bool& val)
  {
    cf_assert( node.is_valid() );
    val = from_str<bool>(node.content->value());
  }

  template <>
  Common_API void to_value<int> (XmlNode& node, int& val)
  {
    cf_assert( node.is_valid() );
    val = from_str<int>(node.content->value());
  }

  template <>
  Common_API void to_value<Uint> (XmlNode& node, Uint& val)
  {
    cf_assert( node.is_valid() );
    val = from_str<Uint>(node.content->value());
  }

  template <>
  Common_API void to_value<Real> (XmlNode& node, Real& val)
  {
    cf_assert( node.is_valid() );
    val = from_str<Real>(node.content->value());
  }

  template <>
  Common_API void to_value<std::string> (XmlNode& node, std::string& val)
  {
    cf_assert( node.is_valid() );
    val = node.content->value();
  }

  template <>
  Common_API void to_value<URI> (XmlNode& node, URI& val)
  {
    cf_assert( node.is_valid() );
    val = from_str<URI>(node.content->value());
  }

////////////////////////////////////////////////////////////////////////////////

  template <>
  Common_API bool to_value<bool> (XmlNode& node)
  {
    return from_str<bool>(node.content->value());
  }

  template <>
  Common_API int to_value<int> (XmlNode& node)
  {
    return from_str<int>(node.content->value());
  }

  template <>
  Common_API Uint to_value<Uint> (XmlNode& node)
  {
    return from_str<Uint>(node.content->value());
  }

  template <>
  Common_API Real to_value<Real> (XmlNode& node)
  {
    return from_str<Real>(node.content->value());
  }

  template <>
  Common_API std::string to_value<std::string> (XmlNode& node)
  {
    return node.content->value();
  }

  template <>
  Common_API URI to_value<URI> (XmlNode& node)
  {
    return from_str<URI>(node.content->value());
  }

////////////////////////////////////////////////////////////////////////////////

  template <>
      Common_API std::string from_string_value<std::string> ( const std::string & str )
  {
    return str;
  }

  template <typename T>
      T from_string_value ( const std::string & str)
  {
    return from_str<T>(str);
  }

  Common_TEMPLATE template bool from_string_value<bool>( const std::string & str );
  Common_TEMPLATE template int from_string_value<int>( const std::string & str );
  Common_TEMPLATE template Uint from_string_value<Uint>( const std::string & str );
  Common_TEMPLATE template Real from_string_value<Real>( const std::string & str );
  Common_TEMPLATE template URI from_string_value<URI>( const std::string & str );

////////////////////////////////////////////////////////////////////////////////

} // XML
} // Common
} // CF
