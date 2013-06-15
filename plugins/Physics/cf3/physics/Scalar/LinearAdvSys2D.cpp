// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "cf3/common/Builder.hpp"

#include "LinearAdvSys2D.hpp"

namespace cf3 {
namespace physics {
namespace Scalar {

////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Scalar::LinearAdvSys2D,
                           physics::Variables,
                           LibScalar >
                           Builder_LinearAdvSys2D;

LinearAdvSys2D::LinearAdvSys2D(const std::string& name) : VariablesT<LinearAdvSys2D>(name)
{
  description().set_variables("U0,U1",MODEL::_ndim);
}

LinearAdvSys2D::~LinearAdvSys2D() {}

////////////////////////////////////////////////////////////////////////////////////

} // Scalar
} // physics
} // cf3
