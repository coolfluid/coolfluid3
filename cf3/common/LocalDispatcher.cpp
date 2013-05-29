// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Component.hpp"
#include "common/Core.hpp"

#include "common/XML/SignalFrame.hpp"
#include "common/XML/SignalOptions.hpp"

#include "common/LocalDispatcher.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////

void LocalDispatcher::dispatch_signal(const std::string & target, const URI & receiver,
                                      SignalArgs & args)
{
  Handle<Component> comp = Core::instance().root().access_component( receiver );
  cf3_assert(is_not_null(comp));
  args.options().flush();
  comp->call_signal( target, args );
}

/////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
