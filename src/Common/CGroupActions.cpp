// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CGroupActions.hpp"
#include "Common/Foreach.hpp"
#include "Common/FindComponents.hpp"
#include "Common/CBuilder.hpp"
#include "Common/LibCommon.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

ComponentBuilder < CGroupActions, CAction, LibCommon > CGroupActions_Builder;

///////////////////////////////////////////////////////////////////////////////////////

CGroupActions::CGroupActions ( const std::string& name ) :  CAction(name)
{
}

////////////////////////////////////////////////////////////////////////////////

void CGroupActions::execute()
{
  // call all actions and action links inside this component
  boost_foreach(Component& child, children())
  {
    if (CAction::Ptr action = child.follow()->as_ptr<CAction>())
      action->execute();
  }
}

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

////////////////////////////////////////////////////////////////////////////////////

