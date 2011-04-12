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

ComponentBuilder < CGroupActions, CGroupActions, LibCommon > CGroupActions_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
CGroupActions::CGroupActions ( const std::string& name ) :  CAction(name)
{
}

////////////////////////////////////////////////////////////////////////////////

void CGroupActions::execute()
{
  boost_foreach(CGroupActions& action, find_components<CGroupActions>(*this))
    action.execute();
}

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

////////////////////////////////////////////////////////////////////////////////////

