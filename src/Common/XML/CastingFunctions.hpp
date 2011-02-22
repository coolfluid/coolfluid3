// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_XML_CastingFunctions_hpp
#define CF_Common_XML_CastingFunctions_hpp

/////////////////////////////////////////////////////////////////////////////////

#include "Common/XML/XmlNode.hpp"

/////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace XML {

/////////////////////////////////////////////////////////////////////////////////

  /// converts the value inside the xml node to the type
  template < typename T>
  void to_value (XmlNode& node, T& val);

  template < typename T>
  T to_value (XmlNode& node);

  ////////////////////////////////////////////////////////////////////////////////

} // XML
} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_XML_CastFunctions_hpp
