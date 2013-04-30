// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"
#include "common/Group.hpp"

#include "mesh/actions/LibActions.hpp"
#include "mesh/actions/LoadBalance.hpp"

namespace cf3 {
namespace mesh {
namespace actions {

using namespace cf3::common;

cf3::common::RegistLibrary<LibActions> libActions;

const char * balancer_name = "LoadBalancer";

} // actions
} // mesh
} // cf3
