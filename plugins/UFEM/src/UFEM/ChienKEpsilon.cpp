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
  KEpsilonBase(name)
{
  trigger_set_expression();
}

void ChienKEpsilon::do_set_expressions(LSSActionUnsteady& lss_action, solver::actions::Proto::ProtoAction& update_nut, FieldVariable<2, VectorField>& u)
{
  Real& dt = lss_action.dt();
  Real& invdt = lss_action.invdt();

  // The length scale helper function
  const auto nut_update = make_lambda([&](const Real yplus, const Real k_in, const Real epsilon)
  {
    const Real f_mu = 1. - ::exp(-0.0115*yplus);
    const Real k = std::max(0., k_in);
    const Real cmu_k2 = m_c_mu * k*k;
    const Real result = f_mu*(cmu_k2 <= sqrt(k)*epsilon*m_l_max ? (epsilon <= 0. ? 0. : cmu_k2 / epsilon) : m_l_max * sqrt(k));

    if(result < m_nu_lam*m_minimal_viscosity_ratio)
    {
      return m_nu_lam*m_minimal_viscosity_ratio;
    }
    return result;
  });

  // Correct yplus in case it is 0
  const auto yp = make_lambda([&](const Real yplus, const Real d)
  {
    if(yplus > 1000. || yplus == 0 && d > 0.)
    {
      return 10000.;
    }

    return yplus;
  });

  update_nut.set_expression(nodes_expression(group
  (
    nu_eff = m_nu_lam + nut_update(yp(yplus, d), k, epsilon)
  )));

  // Helper to get the turbulent viscosity
  const auto nut = make_lambda([&](const Real nu_eff)
  {
    return nu_eff - m_nu_lam + m_nu_lam*m_minimal_viscosity_ratio;
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
  const auto f2 = make_lambda([this](const Real k_in, const Real e)
  {
    if(e < 1e-30)
    {
      return 1.;
    }
    const Real k = std::max(0., k_in);
    const Real Re_t = k*k / (m_nu_lam*e);
    return 1. - 0.4/1.8*::exp(-(Re_t*Re_t)/36.);
  });

  const auto d2 = make_lambda([](const Real d)
  {
    if(d > 0.)
    {
      return d*d;
    }
    return 1e-10;
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
                 + transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * (gamma(k, nut(nu_eff), epsilon, yp(yplus, d)) + lit(2.)*m_nu_lam/(d2(d))) * N(k), // sink and wall distance terms
        _T(k,k) +=  transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * N(k),
        _a[k] += transpose(N(k) + (tau_su*u + cw.apply(u, k))*nabla(k)) * (0.5*nut(nu_eff)) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i))), // Production

        _A(epsilon,epsilon) += transpose(N(epsilon) + (tau_su*u + cw.apply(u, epsilon))*nabla(epsilon)) * u * nabla(epsilon) + (m_nu_lam + nut(nu_eff) / m_sigma_epsilon) * transpose(nabla(epsilon)) * nabla(epsilon) // Advection and diffusion
                       + transpose(N(epsilon) + (tau_su*u + cw.apply(u, epsilon))*nabla(epsilon))*N(epsilon)*(m_c_epsilon_2 * f2(k, epsilon) * gamma(k, nut(nu_eff), epsilon, yp(yplus, d)) + lit(2.)*m_nu_lam / (d2(d))*_exp(-yp(yplus, d)/2.)), // sink and wall distance terms
        _T(epsilon,epsilon) +=  transpose(N(epsilon) + (tau_su*u + cw.apply(u, epsilon))*nabla(epsilon)) * N(epsilon),
        _a[epsilon] += transpose(N(epsilon) + (tau_su*u + cw.apply(u, epsilon))*nabla(epsilon)) * gamma(k, nut(nu_eff), epsilon, yp(yplus, d)) * lit(m_c_epsilon_1) * (0.5*nut(nu_eff)) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i)))
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
