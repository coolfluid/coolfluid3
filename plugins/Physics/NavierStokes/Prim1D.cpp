// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "Prim1D.hpp"

namespace cf3 {
namespace physics {
namespace NavierStokes {

////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < NavierStokes::Prim1D,
                           physics::Variables,
                           LibNavierStokes >
                           Builder_Prim1D;

Prim1D::Prim1D(const std::string& name) : VariablesT<Prim1D>(name)
{
  description().set_variables("Rho,U,P",MODEL::_ndim);
}

Prim1D::~Prim1D() {}

////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // physics
} // cf3
