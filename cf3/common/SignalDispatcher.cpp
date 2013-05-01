// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/URI.hpp"
#include "common/XML/SignalFrame.hpp"

#include "common/SignalDispatcher.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

//////////////////////////////////////////////////////////////////////////////

void SignalDispatcher::dispatch_empty_signal( const std::string & target,
                                              const URI & receiver)
{
  XML::SignalFrame frame( target, URI(), receiver );
  dispatch_signal( target, receiver, frame );
}

//////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

