// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"

#include "Math/LibMath.hpp"

namespace cf3 {
namespace Math {

cf3::common::RegistLibrary<LibMath> libMath;

////////////////////////////////////////////////////////////////////////////////

void LibMath::initiate_impl()
{
}

void LibMath::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Math
} // cf3
