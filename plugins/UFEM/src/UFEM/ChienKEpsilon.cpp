// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "ChienKEpsilon.hpp"

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

common::ComponentBuilder < ChienKEpsilon, common::Action, LibUFEM > ChienKEpsilon_builder;

ChienKEpsilon::ChienKEpsilon(const std::string& name) :
  solver::Action(name)
{
  options().add("velocity_tag", "navier_stokes_solution")
    .pretty_name("Velocity Tag")
    .description("Tag for the velocity field")
    .attach_trigger(boost::bind(&ChienKEpsilon::trigger_set_expression, this));

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

  create_component<ProtoAction>("UpdateNut");

  auto lss = create_component<LSSActionUnsteady>("LSS");
  lss->set_solution_tag("ke_solution");
  lss->create_component<math::LSS::ZeroLSS>("ZeroLSS");
  lss->create_component<ProtoAction>("Assembly");
  lss->create_component<BoundaryConditions>("BoundaryConditions")->set_solution_tag("ke_solution");
  lss->get_child("BoundaryConditions")->mark_basic();
  lss->create_component<math::LSS::SolveLSS>("SolveLSS");
  lss->create_component<ProtoAction>("Update");
  lss->mark_basic();

  trigger_set_expression();
}

void ChienKEpsilon::trigger_set_expression()
{
  // The code will only be active for these element types
  boost::mpl::vector2<mesh::LagrangeP1::Quad2D, mesh::LagrangeP1::Triag2D> allowed_elements;
  const std::string velocity_tag = options().value<std::string>("velocity_tag");

  FieldVariable<0, ScalarField> k("k", "ke_solution");
  FieldVariable<1, ScalarField> epsilon("epsilon", "ke_solution");
  FieldVariable<2, VectorField> u("Velocity", velocity_tag);
  FieldVariable<3, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity"); // This is the viscosity that needs to be modified to be visible in NavierStokes
  FieldVariable<4, ScalarField> d("wall_distance", "wall_distance");
  FieldVariable<5, ScalarField> yplus("yplus", "yplus");

  PhysicsConstant nu_lam("kinematic_viscosity");

  auto lss_action = Handle<LSSActionUnsteady>(get_child("LSS"));
  Real& dt = lss_action->dt();
  Real& invdt = lss_action->invdt();

  // The length scale helper function
  const auto nut_update = make_lambda([&](const Real yplus, const Real k_in, const Real epsilon, const Real nu_l)
  {
    const Real f_mu = 1. - ::exp(-0.0115*yplus);
    const Real k = std::max(0., k_in);
    const Real cmu_k2 = m_c_mu * k*k;
    const Real result = cmu_k2 <= sqrt(k)*epsilon*m_l_max ? (epsilon <= 0. ? 0. : cmu_k2 / epsilon) : m_l_max * sqrt(k);

    if(result < nu_l*m_minimal_viscosity_ratio)
    {
      return f_mu*nu_l*m_minimal_viscosity_ratio;
    }
    return f_mu*result;
  });

  Handle<ProtoAction>(get_child("UpdateNut"))->set_expression(nodes_expression(group
  (
    nu_eff = nu_lam + nut_update(yplus, k, epsilon, nu_lam)
  )));

  // Helper to get the turbulent viscosity
  const auto nut = make_lambda([&](const Real nu, const Real nu_eff)
  {
    return nu_eff - nu;
  });

  // Linearization parameter gamma = epsilon / k:
  const auto gamma = make_lambda([&](const Real k, const Real nu_t, const Real epsilon, const Real yplus)
  {
    const Real f_mu = 1. - ::exp(-0.0115*yplus);
    const Real gamma_mu = f_mu * m_c_mu * std::max(k,0.) / nu_t;
    if(k > 0. && epsilon > 0.)
    {
      return std::min(epsilon/k, gamma_mu);
    }

    return gamma_mu;
  });

  // The f2 helper function
  const auto f2 = make_lambda([](const Real k_in, const Real e, const Real nu)
  {
    if(e < 1e-30)
    {
      return 1.;
    }
    const Real k = std::max(0., k_in);
    const Real Re_t = k*k / (nu*e);
    return 1. - 0.4/1.8*::exp(-(Re_t*Re_t)/36.);
  });

  Handle<ProtoAction>(lss_action->get_child("Assembly"))->set_expression(
  elements_expression
  (
    allowed_elements,
    group
    (
      _A = _0, _T = _0, _a = _0,
      compute_tau.apply( u, nu_eff, lit(dt), lit(tau_su)),
      element_quadrature
      (
        _A(k,k) += transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * u * nabla(k) + (nu_lam + nut(nu_lam, nu_eff) / m_sigma_k) * transpose(nabla(k)) * nabla(k) // Advection and diffusion
                 + transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * (gamma(k, nut(nu_lam, nu_eff), epsilon, yplus) + lit(2.)*nu_lam/(d*d)) * N(k), // wall distance term
        _T(k,k) +=  transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * N(k),
        _a[k] += transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * (0.5*nut(nu_lam, nu_eff)) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i))), // Production
        _A(epsilon,epsilon) += transpose(N(epsilon) + (tau_su*u + cw.apply(u, epsilon))*nabla(epsilon)) * u * nabla(epsilon) + (nu_lam + nut(nu_lam, nu_eff) / m_sigma_epsilon) * transpose(nabla(epsilon)) * nabla(epsilon) // Advection and diffusion
                       + transpose(N(epsilon) + (tau_su*u + cw.apply(u, epsilon))*nabla(epsilon))*N(epsilon)*(m_c_epsilon_2 * f2(k, epsilon, nu_lam) * gamma(k, nut(nu_lam, nu_eff), epsilon, yplus) + lit(2.)*nu_lam / (d*d)*_exp(-yplus/2.)), // wall distance term
        _T(epsilon,epsilon) +=  transpose(N(epsilon) + (tau_su*u + cw.apply(u, epsilon))*nabla(epsilon)) * N(epsilon),
        _a[epsilon] += transpose(N(epsilon) + (tau_su*u + cw.apply(u, epsilon))*nabla(epsilon)) * gamma(k, nut(nu_lam, nu_eff), epsilon, yplus) * lit(m_c_epsilon_1) * (0.5*nut(nu_lam, nu_eff)) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i)))
      ),
      lss_action->system_matrix += invdt * _T + m_theta * _A,
      lss_action->system_rhs += -_A * _x + _a
    )
  ));

  Handle<ProtoAction>(lss_action->get_child("Update"))->set_expression(nodes_expression(group
  (
    k += lss_action->solution(k),
    epsilon += lss_action->solution(epsilon)
  )));
}

void ChienKEpsilon::execute()
{
  auto lss_action = Handle<LSSActionUnsteady>(get_child("LSS"));
  Handle<ProtoAction> update_nut(get_child("UpdateNut"));
  update_nut->execute();
  for(Uint i = 0; i != 2; ++i)
  {
    lss_action->execute();
  }
  update_nut->execute();
}

void ChienKEpsilon::on_regions_set()
{
  get_child("UpdateNut")->options().set("regions", options()["regions"].value());
  get_child("LSS")->options().set("regions", options()["regions"].value());
  access_component("LSS/BoundaryConditions")->options().set("regions", options()["regions"].value());
}

} // UFEM
} // cf3
