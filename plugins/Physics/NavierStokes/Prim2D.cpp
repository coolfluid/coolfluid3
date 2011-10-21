// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "Prim2D.hpp"

namespace cf3 {
namespace Physics {
namespace NavierStokes {

////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < NavierStokes::Prim2D,
                           Physics::Variables,
                           LibNavierStokes >
                           Builder_Prim2D;

Prim2D::Prim2D(const std::string& name) : VariablesT<Prim2D>(name)
{
  description().set_variables("Rho,U[vector],P",MODEL::_ndim);
}

Prim2D::~Prim2D() {}

////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // Physics
} // cf3
