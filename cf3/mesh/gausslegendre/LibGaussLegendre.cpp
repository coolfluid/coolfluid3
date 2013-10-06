// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"

#include "mesh/gausslegendre/LibGaussLegendre.hpp"

namespace cf3 {
namespace mesh {
namespace gausslegendre {

cf3::common::RegistLibrary<LibGaussLegendre> LibGaussLegendre;

} // gausslegendre
} // mesh
} // cf3
