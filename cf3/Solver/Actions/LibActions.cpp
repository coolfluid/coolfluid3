// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"

#include "Solver/Actions/LibActions.hpp"

namespace cf3 {
namespace Solver {
namespace Actions {

cf3::common::RegistLibrary<LibActions> libActions;

////////////////////////////////////////////////////////////////////////////////

void LibActions::initiate_impl()
{
}

void LibActions::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // cf3
