// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Core.hpp"

#include "Actions/LibActions.hpp"

namespace CF {
namespace Actions {

CF::Common::ForceLibRegist<LibActions> libActions;

////////////////////////////////////////////////////////////////////////////////

void LibActions::initiate()
{
}

void LibActions::terminate()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF
