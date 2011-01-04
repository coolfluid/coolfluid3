// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Math/LibMath.hpp"

namespace CF {
namespace Math {

CF::Common::RegistLibrary<LibMath> libMath;

////////////////////////////////////////////////////////////////////////////////

void LibMath::initiate()
{
}

void LibMath::terminate()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Math
} // CF
