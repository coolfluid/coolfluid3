// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "cf3/common/Builder.hpp"

#include "cf3/physics/NavierStokes/Roe1D.hpp"

namespace cf3 {
namespace physics {
namespace NavierStokes {

////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < NavierStokes::Roe1D,
                           physics::Variables,
                           LibNavierStokes >
                           Builder_Roe1D;

Roe1D::Roe1D(const std::string& name) : VariablesT<Roe1D>(name)
{
  description().set_variables("Z0,Z1,Z2",MODEL::_ndim);
}

Roe1D::~Roe1D() {}

////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // physics
} // cf3
