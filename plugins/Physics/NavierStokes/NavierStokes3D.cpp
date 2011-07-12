// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "NavierStokes3D.hpp"

namespace CF {
namespace Physics {
namespace NavierStokes {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

NavierStokes3D::NavierStokes3D( const std::string& name ) : Physics::PhysModel(name)
{
}

NavierStokes3D::~NavierStokes3D()
{
}

////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // Physics
} // CF
