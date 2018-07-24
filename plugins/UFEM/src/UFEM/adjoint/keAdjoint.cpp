// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "keAdjoint.hpp"

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
        _A(ka,ka) +=    transpose(N(ka) - (tau_su*u)*nabla(ka)) * transpose(partial(k,_i)) *m_c_mu*2/m_sigma_k*nabla(ka)*Timescal(k,nu_eff,epsilon)// term 4
                      - transpose(N(ka) - (tau_su*u)*nabla(ka)) * N(ka)*nu_eff*((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i)))/epsilon/Timescal(k,nu_eff,epsilon)//term 5
                      - transpose(N(ka) - (tau_su*u)*nabla(ka)) * ((partial(u[_j], _j)*N(ka) + (u*nabla(ka))))//term 6
                      - transpose(N(ka) - (tau_su*u)*nabla(ka)) * N(ka)* gammaa(ka,epsilona) *m_c_epsilon_2/Timescal(k,nu_eff,epsilon)/Timescal(k,nu_eff,epsilon) //term 3
                      - transpose(N(ka) - (tau_su*u)*nabla(ka)) * N(ka) * gammaa(ka,epsilona)*nu_eff*m_c_epsilon_1/2*((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i)))/epsilon/Timescal(k,nu_eff,epsilon)/Timescal(k,nu_eff,epsilon)//term 2

                      + transpose(nabla(ka))*nabla(ka)*m_c_mu*k*k/m_sigma_k/epsilon,//term7

        _T(ka,ka) +=  transpose(N(ka) - (tau_su*u)*nabla(ka)) * N(ka), // Time
        _a[ka] += -transpose(N(ka) - (tau_su*u)*nabla(ka))*(2*m_c_mu*Timescal(k,nu_eff,epsilon)*(partial(U[_i],_j)*(partial(u[_i],_j)+partial(u[_j],_i)))) // Production
                  -transpose(N(ka) - (tau_su*u)*nabla(ka))* transpose(gradient(epsilon))*gradient(epsilona)*m_c_mu*2/m_sigma_epsilon*Timescal(k,nu_eff,epsilon) //term 1
                  + transpose(N(ka) - (tau_su*u)*nabla(ka)) *2 / 3 * (partial(U[_i],_i)),


        _A(epsilona,epsilona) += - transpose(N(epsilona) - (tau_su*u)*nabla(epsilona)) * transpose(partial(epsilon,_i)) *m_c_mu*Timescal(k,nu_eff,epsilon)*Timescal(k,nu_eff,epsilon)/m_sigma_epsilon*nabla(epsilona)// term 3
                                 - transpose(N(epsilona) - (tau_su*u)*nabla(epsilona)) * ((partial(u[_j], _j)*N(epsilona) + (u*nabla(epsilona)))) //term 4
                                 + transpose(N(epsilona) - (tau_su*u)*nabla(epsilona)) *N(epsilona)*m_c_epsilon_2*2/Timescal(k,nu_eff,epsilon) //term5
                                 + transpose(nabla(epsilona))*(nabla(epsilona))*m_c_mu*k*k/epsilon/m_sigma_epsilon//term6
                                 + transpose(N(epsilona) - (tau_su*u)*nabla(epsilona))* N(epsilona) / gammaa(ka,epsilona)
                                 + transpose(N(epsilona) - (tau_su*u)*nabla(epsilona))* N(epsilona) / gammaa(ka,epsilona) *m_c_mu*k*k/2/epsilon/epsilon/m_sigma_k*((partial(u[_i], _j) + partial(u[_j], _i)) * (partial(u[_i], _j) + partial(u[_j], _i))),// term 2



        _T(epsilona,epsilona) += transpose(N(epsilona) - (tau_su*u)*nabla(epsilona)) * N(epsilona),  //time
        _a[epsilona] += transpose(N(epsilona) - (tau_su*u)*nabla(epsilona))*(m_c_mu*Timescal(k,nu_eff,epsilon)*Timescal(k,nu_eff,epsilon)*(partial(U[_i],_j)*(partial(u[_i],_j)+partial(u[_j],_i))))
                      + (transpose(N(epsilona) - (tau_su*u)*nabla(epsilona))*transpose(gradient(k))*gradient(ka)*m_c_mu*Timescal(k,nu_eff,epsilon)*Timescal(k,nu_eff,epsilon)/m_sigma_k) // term1



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
