// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/GroupActions.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"
#include "common/Builder.hpp"
#include "common/LibCommon.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////////////////

ComponentBuilder < GroupActions, Action, LibCommon > GroupActions_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

GroupActions::GroupActions ( const std::string& name ) :  Action(name) {}


void GroupActions::execute()
{
  // call all actions and action links inside this component

  boost_foreach(Component& child, *this)
  {
    Handle<Action> action(follow_link(child));
    if(is_not_null(action))
      action->execute();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
