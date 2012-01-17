// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "SFDM/navierstokes/BCWallEuler2D.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {
namespace navierstokes {
	
//////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder<BCWallEuler2D,BC,LibNavierStokes> bcwalleuler2d_builder;

/////////////////////////////////////////////////////////////////////////////

} // navierstokes
} // SFDM
} // cf3
