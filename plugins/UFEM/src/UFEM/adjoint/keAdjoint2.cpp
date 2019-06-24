// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "keAdjoint2.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <cmath>
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

common::ComponentBuilder < keAdjoint2, common::Action, LibUFEMAdjoint > keAdjoint2_builder;

keAdjoint2::keAdjoint2(const std::string& name) :
  keAdjointbase(name)
{
  m_c_epsilon_1 = 1.44;
  m_c_epsilon_2 = 1.92;
  trigger_set_expression();
}

void keAdjoint2::do_set_expressions(LSSActionUnsteady& lss_action)//, solver::actions::Proto::ProtoAction& update_nut, FieldVariable<2, VectorField>& u)
{
  Real& dt = lss_action.dt();
  Real& invdt = lss_action.invdt();

  // Helper to get the turbulent viscosity
  const auto nut = make_lambda([&](const Real nu_eff)
  {
    return nu_eff - m_nu_lam;
  });

  // Linearization parameter timescale = k / epsilon
  const auto timescale = make_lambda([&](const Real k, const Real nu_t, const Real epsilon)
  {
      // const Real nu0 = nu_eff-m_c_mu * k*k/epsilon;
      // return std::max(k/epsilon, m_c_tau*std::sqrt(nu0/epsilon));
      const Real t_mu = m_c_tau * std::pow(m_nu_lam/epsilon, 0.5);
      if (k > 0. && epsilon > 0.)
      {
        // CFinfo << "Timescale = " << std::max(k/epsilon, t_mu) << "\n";
        return std::max(k/epsilon, t_mu);
      }
      // CFinfo << "Timescale = " << t_mu << "\n";
      return t_mu;
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
        _A(ka, ka) += - transpose(N(ka) - (tau_su*u + cw.apply(u, ka))*nabla(ka)) * u * nabla(ka)
                    + (m_nu_lam + nut(nu_eff) / m_sigma_k) * transpose(nabla(ka)) * nabla(ka)
                    + transpose(N(ka) - (tau_su*u + cw.apply(u, ka))*nabla(ka)) * lit(2.0) * lit(m_c_mu) / (gamma(k, nut(nu_eff), epsilon) * lit(m_sigma_k)) * transpose(gradient(k)) * nabla(ka)
                    - transpose(N(ka) - (tau_su*u + cw.apply(u, ka))*nabla(ka)) * N(ka) * nut(nu_eff)/epsilon * gamma(k, nut(nu_eff), epsilon) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i))),

        _A(ka, epsilona) += transpose(N(ka) - (tau_su*u + cw.apply(u, ka))*nabla(ka)) * lit(2.0) * lit(m_c_mu) / (m_sigma_epsilon * gamma(k, nut(nu_eff), epsilon)) * transpose(gradient(epsilon)) * nabla(epsilona)
                    - transpose(N(ka) - (tau_su*u + cw.apply(u, ka))*nabla(ka)) * lit(m_c_epsilon_1) * N(epsilona) / epsilon * (gamma(k, nut(nu_eff), epsilon) * gamma(k, nut(nu_eff), epsilon)) * nut(nu_eff) * lit(0.5) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i)))
                    - transpose(N(ka) - (tau_su*u + cw.apply(u, ka))*nabla(ka)) * lit(m_c_epsilon_2) * N(epsilona) * (gamma(k, nut(nu_eff), epsilon) * gamma(k, nut(nu_eff), epsilon)),
        _a[ka] += 
                    //- transpose(N(ka) - (tau_su*u + cw.apply(u, ka))*nabla(ka)) * lit(m_c_mu) / gamma(k, nut(nu_eff), epsilon) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(U[_i], _j) + partial(U[_j], _i)))
                    - transpose(N(ka) - (tau_su*u + cw.apply(u, ka))*nabla(ka)) * lit(m_c_mu) * lit(2.0) / gamma(k, nut(nu_eff), epsilon) * ((partial(u[_i], _j) + partial(u[_j], _i)) * partial(U[_i], _j))
                    
                    
                    + transpose(N(ka) - (tau_su*u + cw.apply(u, ka))*nabla(ka)) * lit(2.0)/lit(3.0) * partial(U[_i], _i),

        _T(ka, ka) += transpose(N(ka) - (tau_su*u + cw.apply(u, ka))*nabla(ka)) * N(ka),


        _A(epsilona, epsilona) += - transpose(N(epsilona) - (tau_su*u + cw.apply(u, epsilona))*nabla(epsilona)) * u * nabla(epsilona) 
                    + (lit(m_nu_lam) + nut(nu_eff) / m_sigma_epsilon) * transpose(nabla(epsilona)) * nabla(epsilona)
                    - transpose(N(epsilona) - (tau_su*u + cw.apply(u, epsilona))*nabla(epsilona)) * lit(m_c_mu) / (gamma(k, nut(nu_eff), epsilon) * gamma(k, nut(nu_eff), epsilon) * m_sigma_epsilon) * transpose(gradient(epsilon)) * nabla(epsilona)
                    + transpose(N(epsilona) - (tau_su*u + cw.apply(u, epsilona))*nabla(epsilona)) * lit(2.0) * lit(m_c_epsilon_2) * N(epsilona) * gamma(k, nut(nu_eff), epsilon),

        _A(epsilona, ka) += - transpose(N(epsilona) - (tau_su*u + cw.apply(u, epsilona))*nabla(epsilona)) * lit(m_c_mu) / (gamma(k, nut(nu_eff), epsilon) * gamma(k, nut(nu_eff), epsilon) * m_sigma_k) * transpose(gradient(k)) * nabla(ka)
                    + transpose(N(epsilona) - (tau_su*u + cw.apply(u, epsilona))*nabla(epsilona)) * N(ka)
                    + transpose(N(epsilona) - (tau_su*u + cw.apply(u, epsilona))*nabla(epsilona)) * N(ka) / epsilon * nut(nu_eff) * lit(0.5) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i))),

        _a[epsilona] += 
                    transpose(N(epsilona) - (tau_su*u + cw.apply(u, epsilona))*nabla(epsilona)) * lit(m_c_mu)  / (gamma(k, nut(nu_eff), epsilon) * gamma(k, nut(nu_eff), epsilon)) * ((partial(u[_i], _j) + partial(u[_j], _i)) * partial(U[_i], _j))
                    
                    ,         

        _T(epsilona, epsilona) += transpose(N(epsilona) - (tau_su*u + cw.apply(u, epsilona))*nabla(epsilona)) * N(epsilona)
        ),
      // _cout << "Matrix A" << "\n" << _A << "\n" << "\n",
      // _cout << "Matrix a " << "\n" << _a << "\n" << "\n",
      lss_action.system_matrix += invdt * _T + m_theta * _A, // invdt() ?????
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
