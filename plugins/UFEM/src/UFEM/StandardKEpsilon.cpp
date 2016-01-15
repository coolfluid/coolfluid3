// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "StandardKEpsilon.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Component.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/PropertyList.hpp"

#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Proto/ElementGradDiv.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "LSSActionUnsteady.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {



using namespace solver::actions::Proto;
using namespace boost::proto;

common::ComponentBuilder < StandardKEpsilon, common::Action, LibUFEM > StandardKEpsilon_builder;

StandardKEpsilon::StandardKEpsilon(const std::string& name) :
  solver::Action(name)
{
  options().add("velocity_tag", "navier_stokes_solution")
    .pretty_name("Velocity Tag")
    .description("Tag for the velocity field")
    .attach_trigger(boost::bind(&StandardKEpsilon::trigger_set_expression, this));

  options().add("theta", m_theta)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .link_to(&m_theta);

  options().add("minimal_viscosity_ratio", m_minimal_viscosity_ratio)
    .pretty_name("Minimal Viscosity Ratio")
    .description("Minimum valua allowed for the ratio nu_t / nu_lam used in the calculations")
    .link_to(&m_minimal_viscosity_ratio);

  options().add("l_max", m_l_max)
    .pretty_name("L Max")
    .description("Maximum value of the mixing length, used to calculate an upper bound on nu_t")
    .link_to(&m_l_max)
    .mark_basic();

  options().add("supg_type", compute_tau.data.op.supg_type_str)
    .pretty_name("SUPG Type")
    .description("Type of computation for the stabilization coefficients.")
    .link_to(&(compute_tau.data.op.supg_type_str))
    .attach_trigger(boost::bind(&ComputeTauImpl::trigger_supg_type, &compute_tau.data.op));

  options().add("u_ref", compute_tau.data.op.u_ref)
    .pretty_name("Reference velocity")
    .description("Reference velocity for the CF2 SUPG method")
    .link_to(&(compute_tau.data.op.u_ref));

  auto k_sys = create_component<LSSActionUnsteady>("K");
  k_sys->set_solution_tag("ke_k");
  k_sys->create_component<ProtoAction>("UpdateNut");
  k_sys->create_component<math::LSS::ZeroLSS>("ZeroLSS");
  k_sys->create_component<ProtoAction>("Assembly");
  k_sys->create_component<BoundaryConditions>("BoundaryConditions")->set_solution_tag("ke_k");
  k_sys->get_child("BoundaryConditions")->mark_basic();
  k_sys->create_component<math::LSS::SolveLSS>("SolveLSS");
  k_sys->create_component<ProtoAction>("Update");
  k_sys->mark_basic();

  auto epsilon_sys = create_component<LSSActionUnsteady>("Epsilon");
  epsilon_sys->set_solution_tag("ke_epsilon");
  epsilon_sys->create_component<math::LSS::ZeroLSS>("ZeroLSS");
  epsilon_sys->create_component<ProtoAction>("Assembly");
  epsilon_sys->create_component<BoundaryConditions>("BoundaryConditions")->set_solution_tag("ke_epsilon");
  epsilon_sys->get_child("BoundaryConditions")->mark_basic();
  epsilon_sys->create_component<math::LSS::SolveLSS>("SolveLSS");
  epsilon_sys->create_component<ProtoAction>("Update");
  epsilon_sys->mark_basic();

  trigger_set_expression();
}

void StandardKEpsilon::trigger_set_expression()
{
  // The code will only be active for these element types
  boost::mpl::vector2<mesh::LagrangeP1::Quad2D, mesh::LagrangeP1::Triag2D> allowed_elements;
  const std::string velocity_tag = options().value<std::string>("velocity_tag");

  FieldVariable<0, ScalarField> k("k", "ke_k");
  FieldVariable<1, ScalarField> epsilon("epsilon", "ke_epsilon");
  FieldVariable<2, VectorField> u("Velocity", velocity_tag);
  FieldVariable<3, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity"); // This is the viscosity that needs to be modified to be visible in NavierStokes
  FieldVariable<4, ScalarField> d("wall_distance", "wall_distance");

  PhysicsConstant nu_lam("kinematic_viscosity");

  auto k_action = Handle<LSSActionUnsteady>(get_child("K"));
  Real& dt = k_action->dt();
  Real& invdt = k_action->invdt();

  // The length scale helper function
  const auto nut_update = make_lambda([&](const Real k_in, const Real epsilon, const Real nu_l, const Real d)
  {
    if(d < 1e-30)
    {
      return nu_l*(1. + m_kappa*m_yplus);
    }
    const Real k = std::max(0., k_in);
    const Real cmu_k2 = m_c_mu * k*k;
    const Real result = cmu_k2 <= sqrt(k)*epsilon*m_l_max ? (epsilon <= 0. ? 0. : cmu_k2 / epsilon) : m_l_max * sqrt(k);

    if(result < nu_l*m_minimal_viscosity_ratio)
    {
      return nu_l*m_minimal_viscosity_ratio;
    }
    return result;
  });

  Handle<ProtoAction>(k_action->get_child("UpdateNut"))->set_expression(nodes_expression(group
  (
    nu_eff = nu_lam + nut_update(k, epsilon, nu_lam, d)
  )));

  // Helper to get the turbulent viscosity
  const auto nut = make_lambda([&](const Real nu, const Real nu_eff)
  {
    return nu_eff - nu;
  });

  // Linearization parameter gamma = epsilon / k:
  const auto gamma = make_lambda([&](const Real k, const Real nu_t, const Real epsilon)
  {
    if(k>1e-30 && epsilon > 1e-30)
    {
      return epsilon/k;
    }

    return m_c_mu * std::max(k,0.) / nu_t;
  });

  const auto wall_weight = make_lambda([&](const Real d)
  {
    if (d < -1.)
      return 0.;

    return 1.;
  });

  const auto u_tau = make_lambda([&](const Real k, const Real u)
  {
    const Real u_k = k >= 0. ? ::pow(m_c_mu, 0.25)*::sqrt(k) : 0.;
    const Real u_tau = std::max(u_k, u/m_yplus);
    return u_tau;
  });

  const auto pow4 = make_lambda([](const Real x)
  {
    return x*x*x*x;
  });

  Handle<ProtoAction>(k_action->get_child("Assembly"))->set_expression(
  elements_expression
  (
    allowed_elements,
    group
    (
      _A = _0, _T = _0, _a = _0,
      compute_tau.apply( u, nu_eff, lit(dt), lit(tau_su)),
      element_quadrature
      (
        _A(k) += transpose(N(k) + tau_su*u*nabla(k)) * u * nabla(k) + (nu_lam + nut(nu_lam, nu_eff) / m_sigma_k) * transpose(nabla(k)) * nabla(k) // Advection and diffusion
                 + transpose(N(k)) * gamma(k, nut(nu_lam, nu_eff), epsilon) * N(k), // sink term
        _T(k) +=  transpose(N(k) + tau_su * u*nabla(k)) * N(k),
        _a[k] += transpose(N(k)) * (wall_weight(d) * (0.5*nut(nu_lam, nu_eff)) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i))) + (lit(1.) - wall_weight(d)) * (pow4(u_tau(k, _norm(u)))/(lit(m_kappa)*lit(m_yplus)*nu_lam))) // Production
      ),
      //apply_weight(_a, nodal_values(d), 0.), // Suppress production on the wall, it will be set equal to epsilon in BC
      k_action->system_matrix += invdt * _T + m_theta * _A,
      k_action->system_rhs += -_A * _x + _a
    )
  ));

  Handle<ProtoAction>(k_action->get_child("Update"))->set_expression(nodes_expression(group
  (
    k += k_action->solution(k)
  )));

  auto epsilon_action = Handle<LSSActionUnsteady>(get_child("Epsilon"));
  Handle<ProtoAction>(epsilon_action->get_child("Assembly"))->set_expression(
  elements_expression
  (
    allowed_elements,
    group
    (
      _A = _0, _T = _0, _a = _0,
      compute_tau.apply( u, nu_eff, lit(dt), lit(tau_su)),
      element_quadrature
      (
        _A(epsilon) += transpose(N(epsilon) + tau_su*u*nabla(epsilon)) * u * nabla(epsilon) + (nu_lam + nut(nu_lam, nu_eff) / m_sigma_epsilon) * transpose(nabla(epsilon)) * nabla(epsilon) // Advection and diffusion
                       + transpose(N(epsilon))*N(epsilon)*(m_c_epsilon_2 * gamma(k, nut(nu_lam, nu_eff), epsilon)), // sink term
        _T(epsilon) +=  transpose(N(epsilon) + tau_su * u*nabla(epsilon)) * N(epsilon),
        _a[epsilon] += transpose(N(epsilon)) * gamma(k, nut(nu_lam, nu_eff), epsilon) * lit(m_c_epsilon_1) * (wall_weight(d) * (0.5*nut(nu_lam, nu_eff)) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i))) + (lit(1.) - wall_weight(d)) * (pow4(u_tau(k, _norm(u)))/(lit(m_kappa)*lit(m_yplus)*nu_lam)))
      ),
      epsilon_action->system_matrix += invdt * _T + m_theta * _A,
      //apply_weight(_a, nodal_values(d), 0.), // Suppress production on the wall, it will be set equal to epsilon in BC
      epsilon_action->system_rhs += -_A * _x + _a
    )
  ));

  Handle<ProtoAction>(epsilon_action->get_child("Update"))->set_expression(nodes_expression(group
  (
    epsilon += epsilon_action->solution(epsilon)
  )));
}

void StandardKEpsilon::execute()
{
  auto k_action = Handle<LSSActionUnsteady>(get_child("K"));
  auto epsilon_action = Handle<LSSActionUnsteady>(get_child("Epsilon"));
  for(Uint i = 0; i != 2; ++i)
  {
    k_action->execute();
    epsilon_action->execute();
  }
  Handle<ProtoAction>(k_action->get_child("UpdateNut"))->execute();
}

void StandardKEpsilon::on_regions_set()
{
  get_child("K")->options().set("regions", options()["regions"].value());
  get_child("Epsilon")->options().set("regions", options()["regions"].value());
  access_component("K/BoundaryConditions")->options().set("regions", options()["regions"].value());
  access_component("Epsilon/BoundaryConditions")->options().set("regions", options()["regions"].value());
}

} // UFEM
} // cf3
