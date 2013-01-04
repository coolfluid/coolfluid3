// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "cf3/common/RegistLibrary.hpp"

#include "cf3/physics/euler/LibEuler.hpp"

namespace cf3 {
namespace physics {
namespace euler {

  using namespace common;

cf3::common::RegistLibrary<LibEuler> LibEuler;

} // euler
} // physics
} // cf3
