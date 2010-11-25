// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Solver/ForwardEuler.hpp"

namespace CF {
namespace Solver {

using namespace Common;

Common::ComponentBuilder < ForwardEuler, CIterativeSolver, LibSolver > ForwardEuler_Builder;

////////////////////////////////////////////////////////////////////////////////

ForwardEuler::ForwardEuler ( const std::string& name  ) :
  CIterativeSolver ( name )
{
  BuildComponent<none>().build(this);
  
  properties()["brief"] = std::string("Iterative Solver component");
  properties()["description"] = std::string("Forward Euler Time Stepper");
}

////////////////////////////////////////////////////////////////////////////////

ForwardEuler::~ForwardEuler()
{
}

////////////////////////////////////////////////////////////////////////////////

void ForwardEuler::do_stuff()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
