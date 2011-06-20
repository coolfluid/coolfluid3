// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Euler/LibEuler.hpp"

namespace CF {
namespace Euler {

  using namespace Common;

CF::Common::RegistLibrary<LibEuler> LibEuler;

////////////////////////////////////////////////////////////////////////////////

void LibEuler::initiate_impl()
{
}

void LibEuler::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Euler
} // CF
