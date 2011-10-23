// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"

#include "LinEuler/LibLinEuler.hpp"

namespace cf3 {
namespace physics {
namespace LinEuler {

  using namespace common;

cf3::common::RegistLibrary<LibLinEuler> LibLinEuler;

////////////////////////////////////////////////////////////////////////////////

void LibLinEuler::initiate_impl()
{
}

void LibLinEuler::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // LinEuler
} // physics
} // cf3

