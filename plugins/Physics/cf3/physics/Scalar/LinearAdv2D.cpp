// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "cf3/common/Builder.hpp"

#include "LinearAdv2D.hpp"

namespace cf3 {
namespace physics {
namespace Scalar {

////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Scalar::LinearAdv2D,
                           physics::Variables,
                           LibScalar >
                           Builder_LinearAdv2D;

LinearAdv2D::LinearAdv2D(const std::string& name) : VariablesT<LinearAdv2D>(name)
{
  description().set_variables("U",MODEL::_ndim);
}

LinearAdv2D::~LinearAdv2D() {}

////////////////////////////////////////////////////////////////////////////////////

} // Scalar
} // physics
} // cf3
