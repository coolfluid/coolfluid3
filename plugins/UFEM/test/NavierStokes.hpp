// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_test_NavierStokes_hpp
#define CF_UFEM_test_NavierStokes_hpp

#include "Solver/Actions/Proto/Expression.hpp"

/// @file Precompiled expressions for the different steps in the Navier-Stokes solver development

namespace CF {
namespace UFEM {

class LinearSolverUnsteady;
class LinearSolver;
class SUPGCoeffs;

/// Parabolic velocity profile as boundary condition
Solver::Actions::Proto::Expression::Ptr parabolic_dirichlet(LinearSolverUnsteady& solver, const RealVector2& u_ref, const Real height);

/// Parabolic velocity profile as initial condition
Solver::Actions::Proto::Expression::Ptr parabolic_field(LinearSolverUnsteady& solver, const RealVector2& u_ref, const Real height);

/// Assembly for the Stokes equations, stabilized with artificial dissipation
Solver::Actions::Proto::Expression::Ptr stokes_artifdiss(LinearSolverUnsteady& solver, SUPGCoeffs& coefs);

/// Assembly for the Stokes equations, stabilized with PSPG
Solver::Actions::Proto::Expression::Ptr stokes_pspg(LinearSolverUnsteady& solver, SUPGCoeffs& coefs);

/// Assembly for the Navier-Stokes equations, stabilized with PSPG
Solver::Actions::Proto::Expression::Ptr navier_stokes_pspg(LinearSolverUnsteady& solver, SUPGCoeffs& coefs);

/// Assembly for the Navier-Stokes equations, stabilized with SUPG
Solver::Actions::Proto::Expression::Ptr navier_stokes_supg(LinearSolverUnsteady& solver, SUPGCoeffs& coefs);

/// Assembly for the Navier-Stokes equations, stabilized with SUPG and bulk viscosity
Solver::Actions::Proto::Expression::Ptr navier_stokes_bulk(LinearSolverUnsteady& solver, SUPGCoeffs& coefs);

  
}
}

#endif // CF_UFEM_test_NavierStokes_hpp
