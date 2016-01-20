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
  KEpsilonBase(name)
{
  m_c_epsilon_1 = 1.44;
  m_c_epsilon_2 = 1.92;
  trigger_set_expression();
}

void StandardKEpsilon::do_set_expressions(LSSActionUnsteady& lss_action, solver::actions::Proto::ProtoAction& update_nut, FieldVariable<2, VectorField>& u)
{
  Real& dt = lss_action.dt();
  Real& invdt = lss_action.invdt();

  // The length scale helper function
  const auto nut_update = make_lambda([&](const Real k_in, const Real epsilon, const Real nu_l, const Real d)
  {
    if(d == 0.)
    {
      return nu_l*m_kappa*m_yplus;
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

  update_nut.set_expression(nodes_expression(group
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
        _A(k,k) += transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * u * nabla(k) + (nu_lam + nut(nu_lam, nu_eff) / m_sigma_k) * transpose(nabla(k)) * nabla(k) // Advection and diffusion
                 + transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * (gamma(k, nut(nu_lam, nu_eff), epsilon)) * N(k), // sink term
        _T(k,k) +=  transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * N(k),
        _a[k] += transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * (0.5*nut(nu_lam, nu_eff)) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i))), // Production

        _A(epsilon,epsilon) += transpose(N(epsilon) + (tau_su*u + cw.apply(u, epsilon))*nabla(epsilon)) * u * nabla(epsilon) + (nu_lam + nut(nu_lam, nu_eff) / m_sigma_epsilon) * transpose(nabla(epsilon)) * nabla(epsilon) // Advection and diffusion
                       + transpose(N(epsilon) + (tau_su*u + cw.apply(u, epsilon))*nabla(epsilon))*N(epsilon)*(m_c_epsilon_2 * gamma(k, nut(nu_lam, nu_eff), epsilon)), // sink term
        _T(epsilon,epsilon) +=  transpose(N(epsilon) + (tau_su*u + cw.apply(u, epsilon))*nabla(epsilon)) * N(epsilon),
        _a[epsilon] += transpose(N(epsilon) + (tau_su*u + cw.apply(u, epsilon))*nabla(epsilon)) * gamma(k, nut(nu_lam, nu_eff), epsilon) * lit(m_c_epsilon_1) * (0.5*nut(nu_lam, nu_eff)) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i)))
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
