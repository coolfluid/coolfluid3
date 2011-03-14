// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  using namespace XML;
  
////////////////////////////////////////////////////////////////////////////////

SignalError::SignalError ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "SignalError")
{}

////////////////////////////////////////////////////////////////////////////////

SignalError::~SignalError() throw()
{}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
