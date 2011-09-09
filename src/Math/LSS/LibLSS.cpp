// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Math/LSS/LibLSS.hpp"

namespace CF {
namespace Math {
namespace LSS {

CF::Common::RegistLibrary<LibLSS> libMath;

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
} // CF
