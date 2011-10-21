// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CGroupActions.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"
#include "common/CBuilder.hpp"
#include "common/LibCommon.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////////////////

ComponentBuilder < CGroupActions, Action, LibCommon > CGroupActions_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

CGroupActions::CGroupActions ( const std::string& name ) :  Action(name) {}


void CGroupActions::execute()
{
  // call all actions and action links inside this component

  boost_foreach(Component& child, children())
  {
    if (Action::Ptr action = child.follow()->as_ptr<Action>())
      action->execute();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // cf3
