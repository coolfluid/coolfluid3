// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "Roe3D.hpp"

namespace cf3 {
namespace Physics {
namespace NavierStokes {

////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < NavierStokes::Roe3D,
                           Physics::Variables,
                           LibNavierStokes >
                           Builder_Roe3D;

Roe3D::Roe3D(const std::string& name) : VariablesT<Roe3D>(name)
{
  description().set_variables("Z0,Z1,Z2,Z3,Z4",MODEL::_ndim);
}

Roe3D::~Roe3D() {}

////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // Physics
} // cf3
