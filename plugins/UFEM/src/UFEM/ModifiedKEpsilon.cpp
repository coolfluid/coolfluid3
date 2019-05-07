// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// Modified k-epsilon turbulence model for the actuator disc.
// Default model proposed by Amina El Kasmi and Christian Masson
// El Kasmi, Amina & Masson, Christian. (2008). An extended model for turbulent flow through horizontal-axis wind turbines. 
// Journal of Wind Engineering and Industrial Aerodynamics. 96. 103-122. 10.1016/j.jweia.2007.03.007. 

// Optionnal model (by setting ren=1) proposed by Huilai Ren et Al. 
// Ren, Huilai & Zhang, Xiaodong & Kang, Shun & Liang, Sichao. (2018).
// Actuator Disc Approach of Wind Turbine Wake Simulation Considering Balance of Turbulence Kinetic Energy.


#include "ModifiedKEpsilon.hpp"

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

common::ComponentBuilder < ModifiedKEpsilon, common::Action, LibUFEM > ModifiedKEpsilon_builder;

ModifiedKEpsilon::ModifiedKEpsilon(const std::string& name) :
  KEpsilonBase(name),
  Ct("thrustCoefficient", "actuator_disk"),
  density_ratio("density_ratio", "density_ratio") // the density ratio should be 1 in a cylinder with the same diameter D as the disc and a height of D/2
{
  options().add("th", m_th)
    .pretty_name("Mesh finesse")
    .description("Mesh finesse")
    .link_to(&m_th)
    .mark_basic();

  options().add("ren", m_ren)
    .pretty_name("Ren and Zhang model")
    .description("Activate the Ren and Zhang model")
    .link_to(&m_ren)
    .mark_basic();

  options().add("Ct", m_Ct)
    .pretty_name("Thrust coefficient")
    .description("Thrust coefficient of the AD")
    .link_to(&m_Ct)
    .mark_basic();

  options().add("u_infty", m_u_infty)
    .pretty_name("Non perturbed speed")
    .description("Speed far away")
    .link_to(&m_u_infty)
    .mark_basic();
  
  m_c_epsilon_1 = 1.176;
  m_c_epsilon_2 = 1.92;
  trigger_set_expression();
}

void ModifiedKEpsilon::do_set_expressions(LSSActionUnsteady& lss_action, solver::actions::Proto::ProtoAction& update_nut, FieldVariable<2, VectorField>& u)
{
  Real& dt = lss_action.dt();
  Real& invdt = lss_action.invdt();

  // The length scale helper function
  const auto nut_update = make_lambda([&](const Real k_in, const Real epsilon, const Real d, const Real nu_eff)
  {
    if(d == 0.) // Boundary value is set in BC
    {
      return nu_eff - m_nu_lam;
    }
    const Real k = std::max(0., k_in);
    const Real cmu_k2 = m_c_mu * k*k;
    const Real result = cmu_k2 <= sqrt(k)*epsilon*m_l_max ? (epsilon <= 0. ? 0. : cmu_k2 / epsilon) : m_l_max * sqrt(k);

    if(result < m_nu_lam*m_minimal_viscosity_ratio)
    {
      return m_nu_lam*m_minimal_viscosity_ratio;
    }
    return result;
  });

  update_nut.set_expression(nodes_expression(group
  (
    nu_eff = m_nu_lam + nut_update(k, epsilon, d, nu_eff)
  )));

  // Helper to get the turbulent viscosity
  const auto nut = make_lambda([&](const Real nu_eff)
  {
    return nu_eff - m_nu_lam;
  });

  // Helper to get the induction factor
  const auto beta_p = make_lambda([&](const Real Ct)
  {
    if (Ct > 0.0)
    {
      Real a = Ct/(4+Ct);
      return m_beta_0 * (0.04 + 0.0144/((1 - a)*(1 - a))) * (1 - a)/(2 * a);
    }
    else return 0.0;
  });

  // Linearization parameter gamma = epsilon / k:
  const auto gamma = make_lambda([&](const Real k, const Real nu_t, const Real epsilon)
  {
    const Real gamma_mu = m_c_mu * std::max(k,0.) / nu_t;
    if(k > 0. && epsilon > 0.)
    {
      return std::min(epsilon/k, gamma_mu);
    }

    return gamma_mu;
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
        _A(k,k) += transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * u * nabla(k) + (m_nu_lam + nut(nu_eff) / m_sigma_k) * transpose(nabla(k)) * nabla(k) // Advection and diffusion
                 + transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * (gamma(k, nut(nu_eff), epsilon)) * N(k), // sink term
        _T(k,k) +=  transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * N(k),
        _a[k] += transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * (0.5*nut(nu_eff)) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i))) // Production
                   + m_ren * (transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * 0.5 * lit(m_u_infty) * lit(m_Ct) / lit(m_th) * (beta_p(m_Ct) * lit(m_u_infty)*lit(m_u_infty) - m_beta_d * k) * density_ratio),  // Source term to correct the AD for the Ren and Zhang model

        _A(epsilon,epsilon) += transpose(N(epsilon) + (tau_su*u + cw.apply(u, epsilon))*nabla(epsilon)) * u * nabla(epsilon) + (m_nu_lam + nut(nu_eff) / m_sigma_epsilon) * transpose(nabla(epsilon)) * nabla(epsilon) // Advection and diffusion
                       + transpose(N(epsilon) + (tau_su*u + cw.apply(u, epsilon))*nabla(epsilon))*N(epsilon)*(m_c_epsilon_2 * gamma(k, nut(nu_eff), epsilon)), // sink term
        _T(epsilon,epsilon) +=  transpose(N(epsilon) + (tau_su*u + cw.apply(u, epsilon))*nabla(epsilon)) * N(epsilon),
        _a[epsilon] += transpose(N(epsilon) + (tau_su*u + cw.apply(u, epsilon))*nabla(epsilon)) * gamma(k, nut(nu_eff), epsilon) * lit(m_c_epsilon_1) * (0.5*nut(nu_eff)) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i)))
                    + density_ratio * transpose(N(epsilon) + (tau_su*u + cw.apply(u, epsilon))*nabla(epsilon)) * m_c_epsilon_4 / k 
                    * (0.5*nut(nu_eff)) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i))) * (0.5*nut(nu_eff)) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i))) // Source term to correct the AD from the El Kasmi and Masson model
      ),
      lss_action.system_matrix += invdt * _T + m_theta * _A,
      lss_action.system_rhs += -_A * _x + _a
    )
  ));

  Handle<ProtoAction>(lss_action.get_child("Update"))->set_expression(nodes_expression(group
  (
    k += lss_action.solution(k),
    epsilon += lss_action.solution(epsilon)
  )));
}

} // UFEM
} // cf3
