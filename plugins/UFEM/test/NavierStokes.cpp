// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_PROTO_MAX_ARITY 10
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10

#include "UFEM/LSSActionUnsteady.hpp"
#include "UFEM/SUPG.hpp"
#include "UFEM/Tags.hpp"

#include "solver/actions/Proto/Expression.hpp"

#include "NavierStokes.hpp"

namespace cf3 {
namespace UFEM {

using namespace solver::actions::Proto;
using boost::proto::lit;

typedef boost::mpl::vector1<mesh::LagrangeP1::Quad2D> AllowedElmsT;

boost::shared_ptr<solver::actions::Proto::ProtoAction> wrap_expression(const boost::shared_ptr<Expression>& expr)
{
  boost::shared_ptr<solver::actions::Proto::ProtoAction> result = common::allocate_component<solver::actions::Proto::ProtoAction>("Assembly");
  result->set_expression(expr);
  return result;
}

boost::shared_ptr<solver::actions::Proto::ProtoAction> stokes_artifdiss(LSSActionUnsteady& solver)
{
  // Expression variables
  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  FieldVariable<1, ScalarField> p("Pressure", "navier_stokes_solution");

  PhysicsConstant rho("density");
  PhysicsConstant mu("dynamic_viscosity");

  return wrap_expression(elements_expression
  (
    AllowedElmsT(),
    group
    (
      _A = _0, _T = _0,
      element_quadrature
      (
        _A(p    , u[_i]) += transpose(N(p)) * nabla(u)[_i],
        _A(p    , p)     += lit(rho) / mu * transpose(nabla(p))*nabla(p),
        _A(u[_i], u[_i]) += mu * transpose(nabla(u))*nabla(u),
        _A(u[_i], p)     += 1./lit(rho) * transpose(N(u))*nabla(p)[_i],
        _T(u[_i], u[_i]) += transpose(N(u))*N(u)
      ),
      solver.system_matrix += solver.invdt() * _T + 0.5 * _A,
      solver.system_rhs += -_A * _x
    )
  ));
}

boost::shared_ptr<solver::actions::Proto::ProtoAction> stokes_pspg(LSSActionUnsteady& solver)
{
  // Expression variables
  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  FieldVariable<1, ScalarField> p("Pressure", "navier_stokes_solution");
  FieldVariable<2, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");

  PhysicsConstant rho("density");
  PhysicsConstant mu("dynamic_viscosity");
  PhysicsConstant u_ref("reference_velocity");

  static Real tau_ps, tau_su, tau_bulk;

  return wrap_expression(elements_expression
  (
    AllowedElmsT(),
    group
    (
      _A = _0, _T = _0,
      compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
      element_quadrature
      (
        _A(p    , u[_i]) +=          transpose(N(p))         * nabla(u)[_i], // Continuity, standard
        _A(p    , p)     += tau_ps * transpose(nabla(p))     * nabla(p),     // Continuity, PSPG
        _A(u[_i], u[_i]) += mu     * transpose(nabla(u))     * nabla(u),     // Diffusion
        _A(u[_i], p)     += 1./lit(rho) * transpose(N(u))         * nabla(p)[_i], // Pressure gradient
        _T(p    , u[_i]) += tau_ps * transpose(nabla(p)[_i]) * N(u),         // Time, PSPG
        _T(u[_i], u[_i]) += transpose(N(u))         * N(u)          // Time, standard
      ),
      solver.system_matrix += solver.invdt() * _T + 0.5 * _A,
      solver.system_rhs += -_A * _x
    )
  ));
}

boost::shared_ptr<solver::actions::Proto::ProtoAction> navier_stokes_pspg(LSSActionUnsteady& solver)
{
  // Expression variables
  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  FieldVariable<1, ScalarField> p("Pressure", "navier_stokes_solution");
  FieldVariable<2, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");

  PhysicsConstant rho("density");
  PhysicsConstant mu("dynamic_viscosity");
  PhysicsConstant u_ref("reference_velocity");

  static Real tau_ps, tau_su, tau_bulk;

  return wrap_expression(elements_expression
  (
    AllowedElmsT(),
    group
    (
      _A = _0, _T = _0,
      compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
      element_quadrature
      (
        _A(p    , u[_i]) +=          transpose(N(p))         * nabla(u)[_i] + tau_ps * transpose(nabla(p)[_i]) * u*nabla(u), // Standard continuity + PSPG for advection
        _A(p    , p)     += tau_ps * transpose(nabla(p))     * nabla(p),     // Continuity, PSPG
        _A(u[_i], u[_i]) += mu     * transpose(nabla(u))     * nabla(u)     + transpose(N(u)) * u*nabla(u),     // Diffusion + advection
        _A(u[_i], p)     += 1./lit(rho) * transpose(N(u))         * nabla(p)[_i], // Pressure gradient
        _T(p    , u[_i]) += tau_ps * transpose(nabla(p)[_i]) * N(u),         // Time, PSPG
        _T(u[_i], u[_i]) += transpose(N(u))         * N(u)          // Time, standard
      ),
      solver.system_matrix += solver.invdt() * _T + 1.0 * _A,
      solver.system_rhs += -_A * _x
    )
  ));
}

boost::shared_ptr<solver::actions::Proto::ProtoAction> navier_stokes_supg(LSSActionUnsteady& solver)
{
  // Expression variables
  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  FieldVariable<1, ScalarField> p("Pressure", "navier_stokes_solution");
  FieldVariable<2, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");

  PhysicsConstant rho("density");
  PhysicsConstant mu("dynamic_viscosity");
  PhysicsConstant u_ref("reference_velocity");

  static Real tau_ps, tau_su, tau_bulk;

  return wrap_expression(elements_expression
  (
    AllowedElmsT(),
    group
    (
      _A = _0, _T = _0,
      compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
      element_quadrature
      (
        _A(p    , u[_i]) +=          transpose(N(p))         * nabla(u)[_i] + tau_ps * transpose(nabla(p)[_i]) * u*nabla(u), // Standard continuity + PSPG for advection
        _A(p    , p)     += tau_ps * transpose(nabla(p))     * nabla(p)/rho,     // Continuity, PSPG
        _A(u[_i], u[_i]) += mu     * transpose(nabla(u))     * nabla(u)/rho     + transpose(N(u) + tau_su*u*nabla(u)) * u*nabla(u),     // Diffusion + advection
        _A(u[_i], p)     += 1./lit(rho) * transpose(N(u) + tau_su*u*nabla(u)) * nabla(p)[_i], // Pressure gradient (standard and SUPG)
        _T(p    , u[_i]) += tau_ps * transpose(nabla(p)[_i]) * N(u),         // Time, PSPG
        _T(u[_i], u[_i]) += transpose(N(u) + tau_su*u*nabla(u))         * N(u)          // Time, standard + SUPG
      ),
      solver.system_matrix += solver.invdt() * _T + 1.0 * _A,
      solver.system_rhs += -_A * _x
    )
  ));
}

}
}
