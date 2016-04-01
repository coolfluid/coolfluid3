// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "keAdjoint.hpp"

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

#include "../LSSActionUnsteady.hpp"
#include "../Tags.hpp"

namespace cf3 {
namespace UFEM {
namespace adjoint{



using namespace solver::actions::Proto;
using namespace boost::proto;

common::ComponentBuilder < keAdjoint, common::Action, LibUFEMAdjoint > keAdjoint_builder;

keAdjoint::keAdjoint(const std::string& name) :
  keAdjointbase(name)
{
  m_c_epsilon_1 = 1.44;
  m_c_epsilon_2 = 1.92;
  trigger_set_expression();
}

void keAdjoint::do_set_expressions(LSSActionUnsteady& lss_action)//, solver::actions::Proto::ProtoAction& update_nut, FieldVariable<2, VectorField>& u)
{
  Real& dt = lss_action.dt();
  Real& invdt = lss_action.invdt();

  // The length scale helper function
  //const auto nut_update = make_lambda([&](const Real k_in, const Real epsilon, const Real d, const Real nu_eff)
  //{
    //if(d == 0.) // Boundary value is set in BC
    //{
     // return nu_eff - m_nu_lam;
    //}
   // const Real k = std::max(0., k_in);
  //  const Real cmu_k2 = m_c_mu * k*k;
//    const Real result = cmu_k2 <= sqrt(k)*epsilon*m_l_max ? (epsilon <= 0. ? 0. : cmu_k2 / epsilon) : m_l_max * sqrt(k);

    //if(result < m_nu_lam*m_minimal_viscosity_ratio)
    //{
     // return m_nu_lam*m_minimal_viscosity_ratio;
    //}
    //return result;
  //});

  //update_nut.set_expression(nodes_expression(group
  //(
    //nu_eff = m_nu_lam + nut_update(k, epsilon, d, nu_eff)
  //)));

  // Helper to get the turbulent viscosity
  //const auto nut = make_lambda([&](const Real nu_eff)
  //{
    //return nu_eff - m_nu_lam;
  //});

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
        _A(ka,ka) +=  (m_nu_lam + nu_eff / m_sigma_k) * transpose(nabla(ka)) * nabla(ka) // Advection and diffusion
          //transpose(N(ka) - (tau_su*u)*nabla(ka)) * transpose(nabla(ka))*nabla(epsilon)*m_c_mu*2*k/epsilon/m_sigma_epsilon
                 // + transpose(N(ka) - (tau_su*u)*nabla(ka)) * N(ka)*gamma(ka, nu_eff, epsilona)*m_c_mu*m_c_epsilon_1/2*((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i)))
                  + transpose(N(ka) - (tau_su*u)*nabla(ka)) * (gamma(ka, nu_eff, epsilona)) * N(ka), // sink term
        _T(ka,ka) +=  transpose(N(ka) - (tau_su*u)*nabla(ka)) * N(ka),
        _a[ka] += transpose(N(ka) - (tau_su*u)*nabla(ka)) * (0.5*nu_eff) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i))), // Production

        _A(epsilona,epsilona) += transpose(N(epsilona) - (tau_su*u)*nabla(epsilona)) * u * nabla(epsilona) + (m_nu_lam + nu_eff / m_sigma_epsilon) * transpose(nabla(epsilona)) * nabla(epsilona) // Advection and diffusion
                       + transpose(N(epsilona) - (tau_su*u)*nabla(epsilona))*N(epsilona)*(m_c_epsilon_2 * gamma(ka, nu_eff, epsilona)), // sink term
        _T(epsilona,epsilona) +=  transpose(N(epsilona) - (tau_su*u)*nabla(epsilona)) * N(epsilona),
        _a[epsilona] += transpose(N(epsilona) - (tau_su*u)*nabla(epsilona)) * gamma(ka, nu_eff, epsilona) * lit(m_c_epsilon_1) * (0.5*nu_eff) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i)))
      ),
      lss_action.system_matrix += invdt * _T + m_theta * _A,
      lss_action.system_rhs += -_A * _x + _a
    )
  ));

  Handle<ProtoAction>(lss_action.get_child("Update"))->set_expression(nodes_expression(group
  (
    ka += lss_action.solution(ka),
    epsilona += lss_action.solution(epsilona)
  )));
}
} // adjoint
} // UFEM
} // cf3
