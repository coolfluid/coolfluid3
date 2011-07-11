// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Physics/Variables.hpp"

#include "LinearAdv3D.hpp"

namespace CF {
namespace Physics {
namespace Scalar {

////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Physics::VariablesT< Scalar::LinearAdv3D >,
                           Physics::Variables,
                           LibScalar >
                           Variables_LinearAdv3D;

////////////////////////////////////////////////////////////////////////////////////

} // Scalar
} // Physics
} // CF
