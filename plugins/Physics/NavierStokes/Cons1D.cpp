// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Cons1D.hpp"

namespace cf3 {
namespace Physics {
namespace NavierStokes {

////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < NavierStokes::Cons1D,
                           Physics::Variables,
                           LibNavierStokes >
                           Builder_Cons1D;

Cons1D::Cons1D(const std::string& name) : VariablesT<Cons1D>(name)
{
  description().set_variables("Rho,RhoU,RhoE",MODEL::_ndim);
}

Cons1D::~Cons1D() {}

////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // Physics
} // cf3
