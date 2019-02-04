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
  //const auto gamma = make_lambda([&](const Real k, const Real nu_t, const Real epsilon)
  //{
   // const Real gamma_mu = m_c_mu * std::max(k,0.0001) / nu_t;
   // if(k > 0. && epsilon > 0.)
    //{
     // return std::min(epsilon/k, gamma_mu);
    //}

    //return gamma_mu;

  //});


//  const auto one_over_gamma = make_lambda([&](const Real k, const Real nu_t, const Real epsilon)
 // {
  //  const Real gamma_mu = m_c_mu * std::max(k,0.0001) / nu_t;
   // if(k > 0. && epsilon > 0.)
    //{
     // return 1./std::max(epsilon/k, gamma_mu);
    //}

    //return 1./gamma_mu;

  //});

    const auto Timescal = make_lambda([&](const Real k, const Real nu_eff, const Real epsilon)
  {
      const Real nu0 = nu_eff-m_c_mu * k*k/epsilon;
      return std::max(k/epsilon, m_c_tau*std::sqrt(nu0/epsilon));


  });


    const auto gammaa = make_lambda([&](const Real ka, const Real epsilona)
  {
     if(m_c_mu*std::pow(ka,1.5) < 100000*epsilona)
     {
         return m_c_mu*ka/ std::max(0.000001,m_c_mu*std::pow(ka,1.5)/epsilona*std::sqrt(ka));
     }
     else
        {
        return m_c_mu*ka/ std::max(0.000001, 100000*std::sqrt(ka));
     }




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
            _A(ka, ka) += - transpose(N(ka) + (tau_su * u) * nabla(ka)) * u * nabla(ka) 
                          + transpose(N(ka) + (tau_su * u) * nabla(ka)) * m_c_mu * 2 * k/epsilon / m_sigma_k * transpose(partial(ka, _i)) * nabla(k)
                          - transpose(nabla(ka)) * nabla(ka) * m_c_mu * k * k/epsilon / m_sigma_k 
                          - transpose(N(ka) + (tau_su * u) * nabla(ka)) * N(ka) * m_c_mu * k/epsilon * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i))) , 
            
            _A(ka, epsilona) +=   transpose(N(ka) + (tau_su * u) * nabla(ka)) * 2 * m_c_mu/m_sigma_k * k/epsilon * transpose(partial(epsilona, _i)) * nabla(epsilon)
                                - transpose(N(ka) + (tau_su * u) * nabla(ka)) * m_c_epsilon_1 * m_c_mu/2 * N(epsilona) * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i)))
                                - transpose(N(ka) + (tau_su * u) * nabla(ka)) * N(epsilona) * m_c_epsilon_2 / (k/epsilon) / (k/epsilon)
                                - transpose(N(epsilona) + (tau_su * u) * nabla(epsilona)) * m_c_mu / m_sigma_k * k/epsilon * k/epsilon * transpose(partial(epsilona, _i)) * nabla(k) 
                                + transpose(N(epsilona) + (tau_su * u) * nabla(epsilona)) * N(ka) * m_c_mu/2 * k/epsilon * k/epsilon * ((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i)))
                                + transpose(N(epsilona) + (tau_su * u) * nabla(epsilona)) * N(ka),
            
            _A(epsilona, epsilona) += - transpose(N(epsilona) + (tau_su * u) * nabla(epsilona)) * u * nabla(epsilona)
                                      - transpose(N(epsilona) + (tau_su * u) * nabla(epsilona)) * m_c_mu/m_sigma_epsilon * k/epsilon * k/epsilon * transpose(partial(epsilona, _i)) * nabla(epsilon) 
                                      - transpose(nabla(epsilona)) * nabla(epsilona) * m_c_mu/m_sigma_epsilon * k * k/epsilon
                                      + transpose(N(epsilona) + (tau_su * u) * nabla(epsilona)) * 2 * N(epsilona) * m_c_epsilon_2 / (k/epsilon),

            _a[ka ] += transpose(N(ka) + (tau_su * u) * nabla(ka)) * (2 * m_c_mu * k/epsilon * (partial(U[_i], _j) * (partial(u[_i], _j) + partial(u[_j], _i)))),
            
            _a[epsilona] += transpose(N(epsilona) + (tau_su * u) * nabla(epsilona)) * (m_c_mu * k/epsilon * k/epsilon * (partial(U[_i], _j) * (partial(u[_i], _j) + partial(u[_j], _i)))),

            _T(ka, ka) += - transpose(N(ka) + (tau_su * u) * nabla(ka)) * N(ka),

            _T(epsilona, epsilona) += - transpose(N(epsilona) + (tau_su * u) * nabla(epsilona)) * N(epsilona) 
        ),
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
