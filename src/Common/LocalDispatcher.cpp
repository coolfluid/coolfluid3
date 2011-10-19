// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Core.hpp"
#include "Common/CRoot.hpp"

#include "Common/XML/SignalFrame.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "Common/LocalDispatcher.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////

void LocalDispatcher::dispatch_signal(const std::string & target, const URI & receiver,
                                      SignalArgs & args)
{
  Component & comp = Core::instance().root().access_component( receiver );
  args.options().flush();
  comp.call_signal( target, args );
}

/////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
