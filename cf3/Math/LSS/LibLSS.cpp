// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"

#include "Math/LSS/LibLSS.hpp"

namespace cf3 {
namespace Math {
namespace LSS {

cf3::common::RegistLibrary<LibLSS> libMath;

////////////////////////////////////////////////////////////////////////////////

void LibLSS::initiate_impl()
{
}

void LibLSS::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // LSS
} // Math
} // cf3
