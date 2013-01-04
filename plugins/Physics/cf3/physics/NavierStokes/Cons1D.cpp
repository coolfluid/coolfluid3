// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "cf3/common/Builder.hpp"

#include "cf3/physics/NavierStokes/Cons1D.hpp"

namespace cf3 {
namespace physics {
namespace NavierStokes {

////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < NavierStokes::Cons1D,
                           physics::Variables,
                           LibNavierStokes >
                           Builder_Cons1D;

Cons1D::Cons1D(const std::string& name) : VariablesT<Cons1D>(name)
{
  description().set_variables("Rho,RhoU,RhoE",MODEL::_ndim);
}

Cons1D::~Cons1D() {}

////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // physics
} // cf3
