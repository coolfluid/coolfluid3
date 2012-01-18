// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "Cons2D.hpp"

namespace cf3 {
namespace physics {
namespace NavierStokes {

////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < NavierStokes::Cons2D,
                           physics::Variables,
                           LibNavierStokes >
                           Builder_Cons2D;

Cons2D::Cons2D(const std::string& name) : VariablesT<Cons2D>(name)
{
  description().set_variables("Rho,RhoU[v],RhoE",MODEL::_ndim);
}

Cons2D::~Cons2D() {}

////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // physics
} // cf3
