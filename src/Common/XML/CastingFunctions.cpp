// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "rapidxml/rapidxml.hpp"

#include "Common/CommonAPI.hpp"
#include "Common/Assertions.hpp"
#include "Common/URI.hpp"
#include "Common/StringConversion.hpp"

#include "Common/XML/CastingFunctions.hpp"

namespace cf3 {
namespace common {
namespace XML {

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

} // XML
} // common
} // cf3
