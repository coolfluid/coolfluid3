// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LinearAdv2D.hpp"

namespace CF {
namespace Physics {
namespace Scalar {

////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Scalar::LinearAdv2D,
                           Physics::Variables,
                           LibScalar >
                           Builder_LinearAdv2D;

LinearAdv2D::LinearAdv2D(const std::string& name) : VariablesT<LinearAdv2D>(name)
{
}

LinearAdv2D::~LinearAdv2D() {}

////////////////////////////////////////////////////////////////////////////////////

} // Scalar
} // Physics
} // CF
