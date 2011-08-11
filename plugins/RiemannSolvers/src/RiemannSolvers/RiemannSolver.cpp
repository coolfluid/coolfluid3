// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "RiemannSolvers/RiemannSolver.hpp"
#include "Common/OptionComponent.hpp"
#include "Solver/State.hpp"

namespace CF {
namespace RiemannSolvers {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

RiemannSolver::RiemannSolver ( const std::string& name  )
: Component(name)
{
  properties()["brief"] = std::string("Riemann Solver");
  properties()["description"] = std::string("Solves the Riemann problem");

  m_options.add_option( OptionComponent<Solver::State>::create("solution_state","Solution State","The component describing the solution state",&m_sol_state) )
      ->add_tag("solution_state");
}

////////////////////////////////////////////////////////////////////////////////

RiemannSolver::~RiemannSolver()
{
}

////////////////////////////////////////////////////////////////////////////////

RealVector RiemannSolver::interface_flux(const RealVector& left, const RealVector& right, const RealVector& normal)
{
  RealVector interface_flux(left.size());
  Real dummy; // not interested in wavespeeds
  solve(
          //input
          left,right,normal,
          //output
          interface_flux,dummy,dummy
        );
  return interface_flux;
}

////////////////////////////////////////////////////////////////////////////////

} // RiemannSolvers
} // CF
