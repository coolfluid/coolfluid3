// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Physics/Variables.hpp"

#include "NavierStokes2D.hpp"

#include "Cons2D.hpp"

namespace CF {
namespace NavierStokes {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

NavierStokes2D::NavierStokes2D( const std::string& name ) : Physics::PhysModel(name)
{
}

NavierStokes2D::~NavierStokes2D()
{
}

////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // CF
