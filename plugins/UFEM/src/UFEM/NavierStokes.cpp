// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Component.hpp"
#include "Common/CBuilder.hpp"

#include "Solver/Actions/Proto/CProtoAction.hpp"
#include "Solver/Actions/Proto/Expression.hpp"

#include "NavierStokes.hpp"
#include "Tags.hpp"
#include "TimeLoop.hpp"

namespace CF {
namespace UFEM {

using namespace Common;
using namespace Solver;
using namespace Solver::Actions::Proto;

ComponentBuilder < NavierStokes, CSolver, LibUFEM > NavierStokes_builder;

NavierStokes::NavierStokes(const std::string& name) : LinearSolverUnsteady(name)
{
  options().add_option< OptionT<Real> >("initial_pressure", 0.)
    ->description("Initial condition for the pressure")
    ->pretty_name("Initial pressure")
    ->link_to(&m_p0);

  options().add_option< OptionT<RealVector> >("initial_velocity")
    ->description("Initial condition for the velocity")
    ->pretty_name("Initial velocity")
    ->link_to(&m_u0);

  options().add_option< OptionT<Real> >("reference_velocity")
    ->description("Reference velocity for the calculation of the stabilization coefficients")
    ->pretty_name("Reference velocity")
    ->link_to(&m_coeffs.u_ref);

  options().add_option< OptionT<Real> >("density", 1.2)
    ->description("Mass density (kg / m^3)")
    ->pretty_name("Density")
    ->link_to(&m_coeffs.rho)
    ->attach_trigger(boost::bind(&NavierStokes::trigger_rho, this));

  options().add_option< OptionT<Real> >("dynamic_viscosity", 1.7894e-5)
    ->description("Dynamic Viscosity (kg / m s)")
    ->pretty_name("Dynamic Viscosity")
    ->link_to(&m_coeffs.mu);


  MeshTerm<0, VectorField> u("Velocity", Tags::solution());
  MeshTerm<1, ScalarField> p("Pressure", Tags::solution());

  *this
    << create_proto_action("InitializePressure", nodes_expression(p = m_p0))
    << create_proto_action("InitializeVelocity", nodes_expression(u = m_u0))
    <<
    ( // Time loop
      create_component<TimeLoop>("TimeLoop")
      << zero_action()
      << create_proto_action
      (
        "Assembly",
        elements_expression
        (
          group <<
          (
            _A = _0, _T = _0,
            compute_tau(u, m_coeffs),
            element_quadrature <<
            (
              _A(p    , u[_i]) +=          transpose(N(p))       * nabla(u)[_i] + m_coeffs.tau_ps * transpose(nabla(p)[_i]) * u*nabla(u), // Standard continuity + PSPG for advection
              _A(p    , p)     += m_coeffs.tau_ps * transpose(nabla(p))     * nabla(p),     // Continuity, PSPG
              _A(u[_i], u[_i]) += m_coeffs.mu     * transpose(nabla(u))     * nabla(u)     + transpose(N(u) + m_coeffs.tau_su*u*nabla(u)) * u*nabla(u),     // Diffusion + advection
              _A(u[_i], p)     += m_coeffs.one_over_rho * transpose(N(u) + m_coeffs.tau_su*u*nabla(u)) * nabla(p)[_i], // Pressure gradient (standard and SUPG)
              _A(u[_i], u[_j]) += m_coeffs.tau_bulk * transpose(nabla(u)[_i]) * nabla(u)[_j], // Bulk viscosity
              _T(p    , u[_i]) += m_coeffs.tau_ps * transpose(nabla(p)[_i]) * N(u),         // Time, PSPG
              _T(u[_i], u[_i]) += transpose(N(u) + m_coeffs.tau_su*u*nabla(u))         * N(u)          // Time, standard
            ),
            system_matrix += invdt() * _T + 1.0 * _A,
            system_rhs += -_A * _b
          )
        )
      )
      << boundary_conditions()
      << solve_action()
      << create_proto_action("IncrementU", nodes_expression(u += solution(u)))
      << create_proto_action("IncrementP", nodes_expression(p += solution(p)))
    );
}

void NavierStokes::trigger_rho()
{
  m_coeffs.one_over_rho = 1. / option("density").value<Real>();
}




} // UFEM
} // CF
