// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"
#include "RungeKutta/LibRungeKutta.hpp"

namespace CF {
namespace RungeKutta {

  using namespace Common;

CF::Common::RegistLibrary<LibRungeKutta> libRungeKutta;

////////////////////////////////////////////////////////////////////////////////

void LibRungeKutta::initiate_impl()
{
}

void LibRungeKutta::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // RungeKutta
} // CF
