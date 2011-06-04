// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "AdvectionDiffusion/State2D.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace AdvectionDiffusion {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < AdvectionDiffusion::State2D, Solver::State, LibAdvectionDiffusion > SolutionState2D_Builder;

////////////////////////////////////////////////////////////////////////////////

} // AdvectionDiffusion
} // CF

////////////////////////////////////////////////////////////////////////////////
