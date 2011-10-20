// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CBuilder.hpp"

#include "LinearAdv1D.hpp"

namespace cf3 {
namespace Physics {
namespace Scalar {

////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Scalar::LinearAdv1D,
                           Physics::Variables,
                           LibScalar >
                           Builder_LinearAdv1D;

LinearAdv1D::LinearAdv1D(const std::string& name) : VariablesT<LinearAdv1D>(name)
{
  description().set_variables("U",MODEL::_ndim);
}

LinearAdv1D::~LinearAdv1D() {}

////////////////////////////////////////////////////////////////////////////////////

} // Scalar
} // Physics
} // cf3
