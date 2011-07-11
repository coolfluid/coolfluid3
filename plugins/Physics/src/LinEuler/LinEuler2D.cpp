// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "LinEuler2D.hpp"

namespace CF {
namespace Physics {
namespace LinEuler {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

LinEuler2D::LinEuler2D( const std::string& name ) : Physics::PhysModel(name)
{
}

LinEuler2D::~LinEuler2D()
{
}

////////////////////////////////////////////////////////////////////////////////

} // LinEuler
} // Physics
} // CF
