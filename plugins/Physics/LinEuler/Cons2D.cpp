// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Cons2D.hpp"

namespace CF {
namespace Physics {
namespace LinEuler {

////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < LinEuler::Cons2D,
                           Physics::Variables,
                           LibLinEuler >
                           Builder_Cons2D;

Cons2D::Cons2D(const std::string& name) : VariablesT<Cons2D>(name)
{
}

Cons2D::~Cons2D() {}

////////////////////////////////////////////////////////////////////////////////////

} // LinEuler
} // Physics
} // CF
