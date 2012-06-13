// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "sdm/lineuler/BCWallNonUniformMeanflow2D.hpp"
#include "solver/Solver.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace lineuler {

//////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder<BCWallNonUniformMeanflow2D,BC,LibLinEuler>BCWallNonUniformMeanflow2D_builder;

/////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3
