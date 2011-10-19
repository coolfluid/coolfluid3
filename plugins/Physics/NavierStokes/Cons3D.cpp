// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Cons3D.hpp"

namespace cf3 {
namespace Physics {
namespace NavierStokes {

////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < NavierStokes::Cons3D,
                           Physics::Variables,
                           LibNavierStokes >
                           Builder_Cons3D;

Cons3D::Cons3D(const std::string& name) : VariablesT<Cons3D>(name)
{
  description().set_variables("Rho,RhoU[v],RhoE",MODEL::_ndim);
}

Cons3D::~Cons3D() {}

////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // Physics
} // cf3
