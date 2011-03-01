// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <sstream>

#include "Common/OSystemLayer.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

OSystemError::OSystemError ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "OSystemError")
{}

////////////////////////////////////////////////////////////////////////////////

OSystemLayer::OSystemLayer() {}

OSystemLayer::~OSystemLayer() {}

////////////////////////////////////////////////////////////////////////////////

void OSystemLayer::execute_command(const std::string& call)
{
  int return_value = system ( call.c_str() );

  if ( return_value == -1)
  {
    std::string msg;
    msg += "Command \'";
    msg += call;
    msg += "\' return error code";
    throw OSystemError ( FromHere(), msg );
  }
}

////////////////////////////////////////////////////////////////////////////////

std::string OSystemLayer::memory_usage_str () const
{
  const CF::Real bytes = memory_usage();

  std::ostringstream out;
  if (  bytes/1024 <= 1 ) {
  out << bytes << " B";
  }
  else if (bytes/1024/1024 <= 1 ) {
    out << bytes/1024 << " KB";
  }
  else if (bytes/1024/1024/1024 <= 1 ) {
    out << bytes/1024/1024 << " MB";
  }
  else {
    out << bytes/1024/1024/1024 << " GB";
  }
  return out.str();
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

