// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_LogStringForwarder_hpp
#define cf3_common_LogStringForwarder_hpp

#include "common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

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

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_LogStringForwarder_hpp
