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

#include "LSSActionUnsteady.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

template<typename AllowedElementsT>
boost::shared_ptr<Expression> generic_ns_assembly(LSSActionUnsteady& solver, SUPGCoeffs& coeffs)
{
  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  FieldVariable<1, ScalarField> p("Pressure", "navier_stokes_solution");
  FieldVariable<2, VectorField> u_adv("AdvectionVelocity", "linearized_velocity"); // The extrapolated advection velocity (n+1/2)
  FieldVariable<3, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");

  return elements_expression
  (
    AllowedElementsT(),
    group

        (
          _A = _0, _T = _0,
          compute_tau(u, coeffs),
          element_quadrature
          (
            _A(p    , u[_i]) += transpose(N(p) + coeffs.tau_ps*u_adv*nabla(p)*0.5) * nabla(u)[_i] + coeffs.tau_ps * transpose(nabla(p)[_i]) * u_adv*nabla(u), // Standard continuity + PSPG for advection
            _A(p    , p)     += coeffs.tau_ps * transpose(nabla(p)) * nabla(p) * coeffs.one_over_rho, // Continuity, PSPG
            _A(u[_i], u[_i]) += nu_eff * transpose(nabla(u)) * nabla(u) + transpose(N(u) + coeffs.tau_su*u_adv*nabla(u)) * u_adv*nabla(u), // Diffusion + advection
            _A(u[_i], p)     += coeffs.one_over_rho * transpose(N(u) + coeffs.tau_su*u_adv*nabla(u)) * nabla(p)[_i], // Pressure gradient (standard and SUPG)
            _A(u[_i], u[_j]) += transpose((coeffs.tau_bulk + 0.33333333333333*nu_eff)*nabla(u)[_i] // Bulk viscosity and second viscosity effect
            + 0.5*u_adv[_i]*(N(u) + coeffs.tau_su*u_adv*nabla(u))) * nabla(u)[_j],  // skew symmetric part of advection (standard +SUPG)
            _T(p    , u[_i]) += coeffs.tau_ps * transpose(nabla(p)[_i]) * N(u), // Time, PSPG
            _T(u[_i], u[_i]) += transpose(N(u) + coeffs.tau_su*u_adv*nabla(u)) * N(u) // Time, standard and SUPG
          ),
          solver.system_matrix += solver.invdt() * _T + 1.0 * _A,
          solver.system_rhs += -_A * _x
        )

    /*(
      _A = _0, _T = _0,
      compute_tau(u, coeffs),
      element_quadrature
      (
        _A(p    , u[_i]) += transpose(N(p) + coeffs.tau_ps*u_adv*nabla(p)*0.5) * nabla(u)[_i] + coeffs.tau_ps * transpose(nabla(p)[_i]) * u_adv*nabla(u), // Standard continuity + PSPG for advection
        _A(p    , p)     += coeffs.tau_ps * transpose(nabla(p)) * nabla(p) * coeffs.one_over_rho, // Continuity, PSPG
        _A(u[_i], u[_i]) += (coeffs.mu + (NU * coeffs.rho)) * transpose(nabla(u)) * nabla(u) * coeffs.one_over_rho + transpose(N(u) + coeffs.tau_su*u_adv*nabla(u)) * u_adv*nabla(u), // Diffusion + advection
        _A(u[_i], p)     += coeffs.one_over_rho * transpose(N(u) + coeffs.tau_su*u_adv*nabla(u)) * nabla(p)[_i], // Pressure gradient (standard and SUPG)
        _A(u[_i], u[_j]) += transpose((coeffs.tau_bulk + 0.33333333333333*boost::proto::lit(coeffs.mu)*coeffs.one_over_rho)*nabla(u)[_i] // Bulk viscosity and second viscosity effect
        + 0.5*u_adv[_i]*(N(u) + coeffs.tau_su*u_adv*nabla(u))) * nabla(u)[_j],  // skew symmetric part of advection (standard +SUPG)
        _T(p    , u[_i]) += coeffs.tau_ps * transpose(nabla(p)[_i]) * N(u), // Time, PSPG
        _T(u[_i], u[_i]) += transpose(N(u) + coeffs.tau_su*u_adv*nabla(u)) * N(u) // Time, standard and SUPG
      ),
      solver.system_matrix += solver.invdt() * _T + 1.0 * _A,
      solver.system_rhs += -_A * _x
    )*/

  );
}

boost::shared_ptr< Expression > ns_assembly_lagrange_p1 ( LSSActionUnsteady& solver, SUPGCoeffs& coeffs )
{
  return generic_ns_assembly< boost::mpl::vector4<mesh::LagrangeP1::Hexa3D, mesh::LagrangeP1::Quad2D, mesh::LagrangeP1::Tetra3D, mesh::LagrangeP1::Triag2D> >(solver, coeffs);
}

boost::shared_ptr< Expression > ns_assembly_quad_hexa_p1 ( LSSActionUnsteady& solver, SUPGCoeffs& coeffs )
{
  return generic_ns_assembly< boost::mpl::vector2<mesh::LagrangeP1::Hexa3D, mesh::LagrangeP1::Quad2D> >(solver, coeffs);
}

} // UFEM
} // cf3
