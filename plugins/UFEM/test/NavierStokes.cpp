// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_PROTO_MAX_ARITY 10
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10

#include "UFEM/LinearSolver.hpp"
#include "UFEM/LinearSolverUnsteady.hpp"
#include "UFEM/NavierStokesOps.hpp"
#include "UFEM/Tags.hpp"


#include "NavierStokes.hpp"

namespace cf3 {
namespace UFEM {

using namespace solver::actions::Proto;

typedef boost::mpl::vector1<mesh::LagrangeP1::Quad2D> AllowedElmsT;

boost::shared_ptr<Expression> stokes_artifdiss(LinearSolverUnsteady& solver, SUPGCoeffs& coefs)
{
  // Expression variables
  MeshTerm<0, VectorField> u("Velocity", UFEM::Tags::solution());
  MeshTerm<1, ScalarField> p("Pressure", UFEM::Tags::solution());

  const Real epsilon = coefs.rho / coefs.mu;
  const Real mu = coefs.mu;

  return elements_expression
  (
    AllowedElmsT(),
    group
    (
      _A = _0, _T = _0,
      element_quadrature
      (
        _A(p    , u[_i]) += transpose(N(p)) * nabla(u)[_i],
        _A(p    , p)     += epsilon * transpose(nabla(p))*nabla(p),
        _A(u[_i], u[_i]) += mu * transpose(nabla(u))*nabla(u),
        _A(u[_i], p)     += 1./coefs.rho * transpose(N(u))*nabla(p)[_i],
        _T(u[_i], u[_i]) += transpose(N(u))*N(u)
      ),
      solver.system_matrix += solver.invdt() * _T + 0.5 * _A,
      solver.system_rhs += -_A * _b
    )
  );
}

boost::shared_ptr<Expression> stokes_pspg(LinearSolverUnsteady& solver, SUPGCoeffs& coefs)
{
  // Expression variables
  MeshTerm<0, VectorField> u("Velocity", UFEM::Tags::solution());
  MeshTerm<1, ScalarField> p("Pressure", UFEM::Tags::solution());

  const Real epsilon = coefs.rho / coefs.mu;
  const Real mu = coefs.mu;

  return elements_expression
  (
    AllowedElmsT(),
    group
    (
      _A = _0, _T = _0,
      compute_tau(u, coefs),
      element_quadrature
      (
        _A(p    , u[_i]) +=          transpose(N(p))         * nabla(u)[_i], // Continuity, standard
        _A(p    , p)     += coefs.tau_ps * transpose(nabla(p))     * nabla(p),     // Continuity, PSPG
        _A(u[_i], u[_i]) += mu     * transpose(nabla(u))     * nabla(u),     // Diffusion
        _A(u[_i], p)     += 1./coefs.rho * transpose(N(u))         * nabla(p)[_i], // Pressure gradient
        _T(p    , u[_i]) += coefs.tau_ps * transpose(nabla(p)[_i]) * N(u),         // Time, PSPG
        _T(u[_i], u[_i]) += transpose(N(u))         * N(u)          // Time, standard
      ),
      solver.system_matrix += solver.invdt() * _T + 0.5 * _A,
      solver.system_rhs += -_A * _b
    )
  );
}

boost::shared_ptr<Expression> navier_stokes_pspg(LinearSolverUnsteady& solver, SUPGCoeffs& coefs)
{
  // Expression variables
  MeshTerm<0, VectorField> u("Velocity", UFEM::Tags::solution());
  MeshTerm<1, ScalarField> p("Pressure", UFEM::Tags::solution());

  const Real epsilon = coefs.rho / coefs.mu;
  const Real mu = coefs.mu;

  return elements_expression
  (
    AllowedElmsT(),
    group
    (
      _A = _0, _T = _0,
      compute_tau(u, coefs),
      element_quadrature
      (
        _A(p    , u[_i]) +=          transpose(N(p))         * nabla(u)[_i] + coefs.tau_ps * transpose(nabla(p)[_i]) * u*nabla(u), // Standard continuity + PSPG for advection
        _A(p    , p)     += coefs.tau_ps * transpose(nabla(p))     * nabla(p),     // Continuity, PSPG
        _A(u[_i], u[_i]) += mu     * transpose(nabla(u))     * nabla(u)     + transpose(N(u)) * u*nabla(u),     // Diffusion + advection
        _A(u[_i], p)     += 1./coefs.rho * transpose(N(u))         * nabla(p)[_i], // Pressure gradient
        _T(p    , u[_i]) += coefs.tau_ps * transpose(nabla(p)[_i]) * N(u),         // Time, PSPG
        _T(u[_i], u[_i]) += transpose(N(u))         * N(u)          // Time, standard
      ),
      solver.system_matrix += solver.invdt() * _T + 1.0 * _A,
      solver.system_rhs += -_A * _b
    )
  );
}

boost::shared_ptr<Expression> navier_stokes_supg(LinearSolverUnsteady& solver, SUPGCoeffs& coefs)
{
  // Expression variables
  MeshTerm<0, VectorField> u("Velocity", UFEM::Tags::solution());
  MeshTerm<1, ScalarField> p("Pressure", UFEM::Tags::solution());

  const Real epsilon = coefs.rho / coefs.mu;
  const Real mu = coefs.mu;

  return elements_expression
  (
    AllowedElmsT(),
    group
    (
      _A = _0, _T = _0,
      compute_tau(u, coefs),
      element_quadrature
      (
        _A(p    , u[_i]) +=          transpose(N(p))         * nabla(u)[_i] + coefs.tau_ps * transpose(nabla(p)[_i]) * u*nabla(u), // Standard continuity + PSPG for advection
        _A(p    , p)     += coefs.tau_ps * transpose(nabla(p))     * nabla(p),     // Continuity, PSPG
        _A(u[_i], u[_i]) += mu     * transpose(nabla(u))     * nabla(u)     + transpose(N(u) + coefs.tau_su*u*nabla(u)) * u*nabla(u),     // Diffusion + advection
        _A(u[_i], p)     += 1./coefs.rho * transpose(N(u) + coefs.tau_su*u*nabla(u)) * nabla(p)[_i], // Pressure gradient (standard and SUPG)
        _T(p    , u[_i]) += coefs.tau_ps * transpose(nabla(p)[_i]) * N(u),         // Time, PSPG
        _T(u[_i], u[_i]) += transpose(N(u) + coefs.tau_su*u*nabla(u))         * N(u)          // Time, standard + SUPG
      ),
      solver.system_matrix += solver.invdt() * _T + 1.0 * _A,
      solver.system_rhs += -_A * _b
    )
  );
}

boost::shared_ptr<Expression> navier_stokes_bulk(LinearSolverUnsteady& solver, SUPGCoeffs& coefs)
{
    // Expression variables
  MeshTerm<0, VectorField> u("Velocity", UFEM::Tags::solution());
  MeshTerm<1, ScalarField> p("Pressure", UFEM::Tags::solution());

  const Real epsilon = coefs.rho / coefs.mu;
  const Real mu = coefs.mu;

  return elements_expression
  (
    AllowedElmsT(),
    group
    (
      _A = _0, _T = _0,
      compute_tau(u, coefs),
      element_quadrature
      (
        _A(p    , u[_i]) +=          transpose(N(p))         * nabla(u)[_i] + coefs.tau_ps * transpose(nabla(p)[_i]) * u*nabla(u), // Standard continuity + PSPG for advection
        _A(p    , p)     += coefs.tau_ps * transpose(nabla(p))     * nabla(p),     // Continuity, PSPG
        _A(u[_i], u[_i]) += mu     * transpose(nabla(u))     * nabla(u)     + transpose(N(u) + coefs.tau_su*u*nabla(u)) * u*nabla(u),     // Diffusion + advection
        _A(u[_i], p)     += 1./coefs.rho * transpose(N(u) + coefs.tau_su*u*nabla(u)) * nabla(p)[_i], // Pressure gradient (standard and SUPG)
        _A(u[_i], u[_j]) += transpose(coefs.tau_bulk*nabla(u)[_i] + 0.5*u[_i]*N(u)) * nabla(u)[_j], // Bulk viscosity and skew symmetric part
        _T(p    , u[_i]) += coefs.tau_ps * transpose(nabla(p)[_i]) * N(u),         // Time, PSPG
        _T(u[_i], u[_i]) += transpose(N(u) + coefs.tau_su*u*nabla(u))         * N(u)          // Time, standard
      ),
      solver.system_matrix += solver.invdt() * _T + 1.0 * _A,
      solver.system_rhs += -_A * _b
    )
  );
}

}
}
