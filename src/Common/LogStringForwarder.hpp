// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_LogStringForwarder_hpp
#define CF_Common_LogStringForwarder_hpp

#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

class Common_API LogStringForwarder
{
  public:

  /// Empty constructor
  LogStringForwarder();

  virtual ~LogStringForwarder();

  /// @todo missing API documentation
  virtual void message(const std::string & str) = 0;

}; // class LogStringForwarder

////////////////////////////////////////////////////////////////////////////////

} //  Common
} //  CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_LogStringForwarder_hpp
