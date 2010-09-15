// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_SetupObject_hpp
#define CF_Common_SetupObject_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// Class defining an object that can be setupo and unsetup
/// @author Tiago Quintino
class Common_API SetupObject {
public: // functions

  /// Constructor
  explicit SetupObject();

  /// Virtual destructor
  virtual ~SetupObject();

  /// Checks if the object is setup
  virtual bool isSetup();

  /// Setup the object
  virtual void setup();

  /// Unsetup the object
  virtual void unsetup();

private: // data

  /// setup flag
  bool m_setup;

}; // end class SetupObject

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_SetupObject_hpp
