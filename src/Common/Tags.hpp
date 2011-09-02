// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_Tags_hpp
#define CF_Common_Tags_hpp

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////////////////

/// Class defines the tags for the math components
/// @author Bart Janssens
class Common_API Tags : public NonInstantiable<Tags> {
public:
  
  static const char * dimension ();

}; // Tags

////////////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

#endif // CF_Common_Tags_hpp
