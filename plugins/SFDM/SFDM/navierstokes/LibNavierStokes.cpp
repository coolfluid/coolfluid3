// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"

#include "SFDM/navierstokes/LibNavierStokes.hpp"

namespace cf3 {
namespace SFDM {
namespace navierstokes {

  using namespace common;

cf3::common::RegistLibrary<LibNavierStokes> LibNavierStokes;


} // navierstokes
} // SFDM
} // cf3

