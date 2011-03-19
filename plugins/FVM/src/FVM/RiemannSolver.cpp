// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "FVM/RiemannSolver.hpp"

namespace CF {
namespace FVM {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

RiemannSolver::RiemannSolver ( const std::string& name  ) 
: Component(name),
  m_g(1.4),
  m_gm1(m_g-1.)
{
  properties()["brief"] = std::string("Riemann Solver");
  properties()["description"] = std::string("Solves the Riemann problem");
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

} // FVM
} // CF
