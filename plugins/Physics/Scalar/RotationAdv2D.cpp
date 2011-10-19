// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CBuilder.hpp"

#include "RotationAdv2D.hpp"

namespace cf3 {
namespace Physics {
namespace Scalar {

////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Scalar::RotationAdv2D,
                           Physics::Variables,
                           LibScalar >
                           Builder_RotationAdv2D;

RotationAdv2D::RotationAdv2D(const std::string& name) : VariablesT<RotationAdv2D>(name)
{
  description().set_variables("U",MODEL::_ndim);
}

RotationAdv2D::~RotationAdv2D() {}

////////////////////////////////////////////////////////////////////////////////////

} // Scalar
} // Physics
} // cf3
