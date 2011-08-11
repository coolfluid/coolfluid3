// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Scalar/LibScalar.hpp"

namespace CF {
namespace Physics {
namespace Scalar {

  using namespace Common;

CF::Common::RegistLibrary<LibScalar> LibScalar;

////////////////////////////////////////////////////////////////////////////////

void LibScalar::initiate_impl()
{
}

void LibScalar::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Scalar
} // Physics
} // CF

