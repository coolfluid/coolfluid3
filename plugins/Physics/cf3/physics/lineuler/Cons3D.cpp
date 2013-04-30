// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "cf3/common/Builder.hpp"

#include "cf3/physics/lineuler/Cons3D.hpp"

namespace cf3 {
namespace physics {
namespace LinEuler {

////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < LinEuler::Cons3D,
                           physics::Variables,
                           LibLinEuler >
                           Builder_Cons3D;

Cons3D::Cons3D(const std::string& name) : VariablesT<Cons3D>(name)
{
  description().set_variables("Rho,Rho0U[v],P",MODEL::_ndim);
}

Cons3D::~Cons3D() {}

////////////////////////////////////////////////////////////////////////////////////

} // LinEuler
} // physics
} // cf3
