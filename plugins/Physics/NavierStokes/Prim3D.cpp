// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "Prim3D.hpp"

namespace cf3 {
namespace physics {
namespace NavierStokes {

////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < NavierStokes::Prim3D,
                           physics::Variables,
                           LibNavierStokes >
                           Builder_Prim3D;

Prim3D::Prim3D(const std::string& name) : VariablesT<Prim3D>(name)
{
  description().set_variables("Rho,U[vector],P",MODEL::_ndim);
}

Prim3D::~Prim3D() {}

////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // physics
} // cf3
