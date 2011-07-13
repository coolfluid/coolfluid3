// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "LinEuler/LibLinEuler.hpp"

namespace CF {
namespace Physics {
namespace LinEuler {

  using namespace Common;

CF::Common::RegistLibrary<LibLinEuler> LibLinEuler;

////////////////////////////////////////////////////////////////////////////////

void LibLinEuler::initiate_impl()
{
}

void LibLinEuler::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // LinEuler
} // Physics
} // CF

