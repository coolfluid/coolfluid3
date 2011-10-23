// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/OptionComponent.hpp"

#include "physics/Variables.hpp"

#include "RiemannSolvers/RiemannSolver.hpp"

namespace cf3 {
namespace RiemannSolvers {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

RiemannSolver::RiemannSolver ( const std::string& name  )
: Component(name)
{
  properties()["brief"] = std::string("Riemann Solver");
  properties()["description"] = std::string("Solves the Riemann problem");

  m_options.add_option( OptionComponent<physics::PhysModel>::create("physical_model",&m_physical_model) )
      ->description("The component describing the physics")
      ->pretty_name("Physical Model");

  m_options.add_option( OptionComponent<physics::Variables>::create("solution_vars",&m_solution_vars) )
      ->description("The component describing the solution")
      ->pretty_name("Solution Variables");
}

////////////////////////////////////////////////////////////////////////////////

RiemannSolver::~RiemannSolver()
{
}

////////////////////////////////////////////////////////////////////////////////

} // RiemannSolvers
} // cf3
