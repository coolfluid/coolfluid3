// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_XML_CastingFunctions_hpp
#define cf3_common_XML_CastingFunctions_hpp

/////////////////////////////////////////////////////////////////////////////////

#include "common/XML/XmlNode.hpp"

/////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
namespace XML {

/////////////////////////////////////////////////////////////////////////////////

  /// converts the value inside the xml node to the type
  template < typename T>
  void to_value (XmlNode& node, T& val);

  template < typename T>
  T to_value (XmlNode& node);

  ////////////////////////////////////////////////////////////////////////////////

} // XML
} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_XML_CastFunctions_hpp
