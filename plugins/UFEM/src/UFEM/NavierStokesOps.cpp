// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_PROTO_MAX_ARITY 10
#ifdef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
#undef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
#endif
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10

#include "NavierStokesOps.hpp"

#include "solver/Time.hpp"

#include "LinearSolverUnsteady.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

boost::shared_ptr<Expression> generic_ns_assembly(LinearSolverUnsteady& solver, SUPGCoeffs& coeffs)
{
  // Elements for which no specialized implementation exists
  boost::mpl::vector2<mesh::LagrangeP1::Hexa3D, mesh::LagrangeP1::Quad2D> generic_elements;

  MeshTerm<0, ScalarField> p("Pressure", Tags::solution());
  MeshTerm<1, VectorField> u("Velocity", Tags::solution());

  MeshTerm<2, VectorField> u_adv("AdvectionVelocity", "linearized_velocity"); // The extrapolated advection velocity (n+1/2)

  return elements_expression
  (
    generic_elements,
    group
    (
      _A = _0, _T = _0,
      compute_tau(u, coeffs),
      element_quadrature
      (
        _A(p    , u[_i]) += transpose(N(p) + coeffs.tau_ps*u_adv*nabla(p)*0.5) * nabla(u)[_i] + coeffs.tau_ps * transpose(nabla(p)[_i]) * u_adv*nabla(u), // Standard continuity + PSPG for advection
        _A(p    , p)     += coeffs.tau_ps * transpose(nabla(p)) * nabla(p) * coeffs.one_over_rho, // Continuity, PSPG
        _A(u[_i], u[_i]) += coeffs.mu * transpose(nabla(u)) * nabla(u) * coeffs.one_over_rho + transpose(N(u) + coeffs.tau_su*u_adv*nabla(u)) * u_adv*nabla(u), // Diffusion + advection
        _A(u[_i], p)     += coeffs.one_over_rho * transpose(N(u) + coeffs.tau_su*u_adv*nabla(u)) * nabla(p)[_i], // Pressure gradient (standard and SUPG)
        _A(u[_i], u[_j]) += transpose((coeffs.tau_bulk + 0.33333333333333*boost::proto::lit(coeffs.mu)*coeffs.one_over_rho)*nabla(u)[_i] // Bulk viscosity and second viscosity effect
        + 0.5*u_adv[_i]*(N(u) + coeffs.tau_su*u_adv*nabla(u))) * nabla(u)[_j],  // skew symmetric part of advection (standard +SUPG)
        _T(p    , u[_i]) += coeffs.tau_ps * transpose(nabla(p)[_i]) * N(u), // Time, PSPG
        _T(u[_i], u[_i]) += transpose(N(u) + coeffs.tau_su*u_adv*nabla(u)) * N(u) // Time, standard and SUPG
      ),
      solver.system_matrix += solver.invdt() * _T + 1.0 * _A,
      solver.system_rhs += -_A * _b
    )
  );
}


} // UFEM
} // cf3
