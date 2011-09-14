// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/OptionComponent.hpp"

#include "Physics/Variables.hpp"

#include "RiemannSolvers/RiemannSolver.hpp"

namespace CF {
namespace RiemannSolvers {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

RiemannSolver::RiemannSolver ( const std::string& name  )
: Component(name)
{
  properties()["brief"] = std::string("Riemann Solver");
  properties()["description"] = std::string("Solves the Riemann problem");

  m_options.add_option( OptionComponent<Physics::Variables>::create("solution_vars",&m_solution_vars) )
      ->description("The component describing the solution")
      ->pretty_name("Solution Variables");
}

////////////////////////////////////////////////////////////////////////////////

RiemannSolver::~RiemannSolver()
{
}

////////////////////////////////////////////////////////////////////////////////

} // RiemannSolvers
} // CF
