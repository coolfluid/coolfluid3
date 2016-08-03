// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "SSTKOmega.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Component.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/PropertyList.hpp"

#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"

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

SSTKOmega::SSTKOmega(const std::string& name) :
  solver::Action(name),
  k("k", "ke_solution"),
  omega("omega", "ke_solution"),
  nu_eff("EffectiveViscosity", "navier_stokes_viscosity"), // This is the viscosity that needs to be modified to be visible in NavierStokes
  d("wall_distance", "wall_distance"),
  yplus("yplus", "yplus")
{
  options().add("velocity_tag", "navier_stokes_solution")
    .pretty_name("Velocity Tag")
    .description("Tag for the velocity field")
    .attach_trigger(boost::bind(&SSTKOmega::trigger_set_expression, this));

  options().add("theta", m_theta)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .link_to(&m_theta);

  options().add("supg_type", compute_tau.data.op.supg_type_str)
    .pretty_name("SUPG Type")
    .description("Type of computation for the stabilization coefficients.")
    .link_to(&(compute_tau.data.op.supg_type_str))
    .attach_trigger(boost::bind(&ComputeTauImpl::trigger_supg_type, &compute_tau.data.op));

  options().add("u_ref", compute_tau.data.op.u_ref)
    .pretty_name("Reference velocity")
    .description("Reference velocity for the CF2 SUPG method")
    .link_to(&(compute_tau.data.op.u_ref));

  link_physics_constant("kinematic_viscosity", m_nu_lam);
  link_physics_constant("density", m_rho);

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
}

SSTKOmega::~SSTKOmega()
{
}

void SSTKOmega::trigger_set_expression()
{
  FieldVariable<2, VectorField> u("Velocity", options().value<std::string>("velocity_tag"));
  do_set_expressions(*Handle<LSSActionUnsteady>(get_child("LSS")), *Handle<ProtoAction>(get_child("UpdateNut")), u);
}

void SSTKOmega::execute()
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

void SSTKOmega::on_regions_set()
{
  get_child("UpdateNut")->options().set("regions", options()["regions"].value());
  get_child("LSS")->options().set("regions", options()["regions"].value());
  access_component("LSS/BoundaryConditions")->options().set("regions", options()["regions"].value());
}

void SSTKOmega::do_set_expressions(LSSActionUnsteady& lss_action, solver::actions::Proto::ProtoAction& update_nut, FieldVariable<2, VectorField>& u)
{
  Real& dt = lss_action.dt();
  Real& invdt = lss_action.invdt();

  const auto nut = make_lambda([&](const Real nu_eff)
  {
    return nu_eff - m_nu_lam;
  });

  const auto F1 = make_lambda([&](const Real phi1, const Real phi2, const Real k, const Real omega, const Real d, const Real d_kx_d_omegax)
  {
    if(k <= 0. || omega <= 0.)
    {
      return phi2;  // F1 = 0
    }
    if(d == 0.)
    {
      return phi1; // F1 = 1
    }

    const Real CD = std::max(2.*m_rho*m_sigma_omega2/omega*d_kx_d_omegax, 1e-10);
    const Real arg1 = std::min(std::max(std::sqrt(k)/(m_beta_s*omega*d), 500.*m_nu_lam/(d*d*omega)), 4.*m_rho*m_sigma_omega2*k/(CD*d*d));
    const Real F1 = std::tanh(arg1*arg1*arg1*arg1);
    return F1*phi1 + (1.-F1)*phi2;
  });

  Handle<ProtoAction>(lss_action.get_child("Assembly"))->set_expression(
  elements_expression
  (
    allowed_elements,
    group
    (
      _A = _0, _T = _0, _a = _0,
      compute_tau.apply( u, nu_eff, lit(dt), lit(tau_su)),
      element_quadrature
      (
        _A(k,k) += transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * u * nabla(k) + (m_nu_lam + nut(nu_eff) * F1(m_sigma_k1, m_sigma_k2, k, omega, d, partial(k,_i)*partial(omega,_i))) * transpose(nabla(k)) * nabla(k) // Advection and diffusion
                 + transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * m_beta_s*omega*N(k), // sink term
        _T(k,k) +=  transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * N(k),
        _a[k] += transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * (0.5*nut(nu_eff)) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i))), // Production

        _A(omega,omega) += transpose(N(omega) + (tau_su*u + cw.apply(u, omega))*nabla(omega)) * u * nabla(omega) + (m_nu_lam + nut(nu_eff) * F1(m_sigma_omega1, m_sigma_omega2, k, omega, d, partial(k,_i)*partial(omega,_i))) * transpose(nabla(omega)) * nabla(omega) // Advection and diffusion
        //                + transpose(N(omega) + (tau_su*u + cw.apply(u, omega))*nabla(omega))*N(omega)*(m_c_epsilon_2 * gamma(k, nut(nu_eff), omega)), // sink term
        // _T(omega,omega) +=  transpose(N(omega) + (tau_su*u + cw.apply(u, omega))*nabla(omega)) * N(omega),
        // _a[omega] += transpose(N(omega) + (tau_su*u + cw.apply(u, omega))*nabla(omega)) * gamma(k, nut(nu_eff), omega) * lit(m_c_epsilon_1) * (0.5*nut(nu_eff)) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i)))
      ),
      lss_action.system_matrix += invdt * _T + m_theta * _A,
      lss_action.system_rhs += -_A * _x + _a
    )
  ));

  Handle<ProtoAction>(lss_action.get_child("Update"))->set_expression(nodes_expression(group
  (
    k += lss_action.solution(k),
    omega += lss_action.solution(omega)
  )));
}

} // UFEM
} // cf3
