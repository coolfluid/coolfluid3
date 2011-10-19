// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Physics/LibPhysics.hpp"

namespace cf3 {
namespace Physics {

cf3::common::RegistLibrary<LibPhysics> libPhysics;

////////////////////////////////////////////////////////////////////////////////

void LibPhysics::initiate_impl()
{
}

void LibPhysics::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Physics
} // cf3
