// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_test_NavierStokes_hpp
#define cf3_UFEM_test_NavierStokes_hpp

#include "solver/actions/Proto/Expression.hpp"

/// @file Precompiled expressions for the different steps in the Navier-Stokes solver development

namespace cf3 {
namespace UFEM {

class LinearSolverUnsteady;
class LinearSolver;
class SUPGCoeffs;

/// Parabolic velocity profile as boundary condition
solver::actions::Proto::Expression::Ptr parabolic_dirichlet(LinearSolverUnsteady& solver, const RealVector2& u_ref, const Real height);

/// Parabolic velocity profile as initial condition
solver::actions::Proto::Expression::Ptr parabolic_field(LinearSolverUnsteady& solver, const RealVector2& u_ref, const Real height);

/// Assembly for the Stokes equations, stabilized with artificial dissipation
solver::actions::Proto::Expression::Ptr stokes_artifdiss(LinearSolverUnsteady& solver, SUPGCoeffs& coefs);

/// Assembly for the Stokes equations, stabilized with PSPG
solver::actions::Proto::Expression::Ptr stokes_pspg(LinearSolverUnsteady& solver, SUPGCoeffs& coefs);

/// Assembly for the Navier-Stokes equations, stabilized with PSPG
solver::actions::Proto::Expression::Ptr navier_stokes_pspg(LinearSolverUnsteady& solver, SUPGCoeffs& coefs);

/// Assembly for the Navier-Stokes equations, stabilized with SUPG
solver::actions::Proto::Expression::Ptr navier_stokes_supg(LinearSolverUnsteady& solver, SUPGCoeffs& coefs);

/// Assembly for the Navier-Stokes equations, stabilized with SUPG and bulk viscosity
solver::actions::Proto::Expression::Ptr navier_stokes_bulk(LinearSolverUnsteady& solver, SUPGCoeffs& coefs);


}
}

#endif // cf3_UFEM_test_NavierStokes_hpp
