// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_Tags_hpp
#define cf3_common_Tags_hpp

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////////////////

/// Class defines the tags for the math components
/// @author Bart Janssens
class Common_API Tags : public NonInstantiable<Tags> {
public:
  
  /// Tag for options related to the dimension of something
  static const char * dimension();
  
  /// Tag to indicate that a component is static
  static const char * static_component();

}; // Tags

////////////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

#endif // cf3_common_Tags_hpp
