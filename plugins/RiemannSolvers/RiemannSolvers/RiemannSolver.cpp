// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

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

  options().add("physical_model",m_physical_model)
      .description("The component describing the physics")
      .pretty_name("Physical Model")
      .link_to(&m_physical_model);

  options().add("solution_vars",m_solution_vars)
      .description("The component describing the solution")
      .pretty_name("Solution Variables")
      .link_to(&m_solution_vars);
}

////////////////////////////////////////////////////////////////////////////////

RiemannSolver::~RiemannSolver()
{
}

////////////////////////////////////////////////////////////////////////////////

} // RiemannSolvers
} // cf3
