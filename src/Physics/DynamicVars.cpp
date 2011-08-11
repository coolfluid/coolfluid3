// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Physics/DynamicVars.hpp"

namespace CF {
namespace Physics {

////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Physics::DynamicVars,
                           Physics::Variables,
                           LibPhysics >
                           Builder_DynamicVars;

DynamicVars::DynamicVars(const std::string &name) : Variables(name) {}

DynamicVars::~DynamicVars() {}

////////////////////////////////////////////////////////////////////////////////////

} // Physics
} // CF
