// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "physics/DynamicVars.hpp"

namespace cf3 {
namespace physics {

////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < physics::DynamicVars,
                           physics::Variables,
                           LibPhysics >
                           Builder_DynamicVars;

DynamicVars::DynamicVars(const std::string &name) : Variables(name) {}

DynamicVars::~DynamicVars() {}

////////////////////////////////////////////////////////////////////////////////////

} // physics
} // cf3
