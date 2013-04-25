// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_semi_implicit_PressureMatrixAssembly_hpp
#define cf3_UFEM_semi_implicit_PressureMatrixAssembly_hpp

#include "NavierStokesSemiImplicit.hpp"

#include <boost/mpl/back_inserter.hpp>
#include <boost/mpl/copy.hpp>

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "UFEM/SUPG.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

using boost::proto::lit;

template<typename ElementsT>
void NavierStokesSemiImplicit::set_elements_expressions( const std::string& name )
{

//   add_component(create_proto_action
//   (
//     action_name,
//     elements_expression
//     (
//       ElementsT(),
//       group
//       (
//   group(_A = _0, _T = _0, M = _0, Ml(u) = _0, Ml(p) = _0),
//         group
//         (
//           compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
//           element_quadrature
//           (
//             _A(p    , u[_i]) += transpose(N(p) + tau_ps*u_adv*nabla(p)*0.5) * nabla(u)[_i] + tau_ps * transpose(nabla(p)[_i]) * u_adv*nabla(u), // Standard continuity + PSPG for advection
//             _A(p    , p)     += tau_ps * transpose(nabla(p)) * nabla(p), // Continuity, PSPG
//             _A(u[_i], u[_i]) += nu_eff * transpose(nabla(u)) * nabla(u) + transpose(N(u) + tau_su*u_adv*nabla(u)) * u_adv*nabla(u), // Diffusion + advection
//             _A(u[_i], p)     += transpose(tau_su*u_adv*nabla(u)) * nabla(p)[_i] - transpose(nabla(u)[_i]) * N(p), // Pressure gradient (standard and SUPG)
//             _A(u[_i], u[_j]) += transpose((tau_bulk + 0.33333333333333*nu_eff)*nabla(u)[_i] // Bulk viscosity and second viscosity effect
//                                 + 0.5*u_adv[_i]*(N(u) + tau_su*u_adv*nabla(u))) * nabla(u)[_j],  // skew symmetric part of advection (standard +SUPG)
//             _T(p    , u[_i]) += tau_ps * transpose(nabla(p)[_i]) * N(u), // Time, PSPG
//             _T(u[_i], u[_i]) += transpose(N(u) + tau_su*u_adv*nabla(u)) * N(u), // Time, standard and SUPG
//             //M(p, u[_i]) += /* More accurate with this term in: */tau_ps * transpose(nabla(p)[_i]) * N(u) + lit(dt()) * (transpose(N(p)) * nabla(u)[_i] /*+ tau_ps * transpose(nabla(p)[_i]) * u_adv*nabla(u)*/),
//             //M(u[_i], p) += -lit(theta) * transpose(nabla(u)[_i]) * N(p)
//             M(p,p) += -lit(theta) * (lit(tau_ps) + lit(dt())) * transpose(nabla(p)) * nabla(p)
//           ),
//   element_quadrature
//   (
//      Ml(u[_i], u[_i]) += transpose(N(u)) * N(u),
//      M(u[_i], u[_i]) += transpose(N(u) + tau_su*u_adv*nabla(u)) * N(u) + lit(theta) * lit(dt()) * (nu_eff * transpose(nabla(u)) * nabla(u)),
//      M(u[_i], u[_j]) += lit(theta) * lit(dt()) * (transpose((tau_bulk + 0.33333333333333*nu_eff)*nabla(u)[_i]) * nabla(u)[_j])
//   )
//         ),
//         //M(p,p) = -_A(p,p), // Minus, easier for the dirichlet conditions afterwards
//         rhs_lss.system_matrix += _A,
//         t_lss.system_matrix += _T,
//         system_matrix += M,
//         lump(Ml),
//         t_lss.system_rhs(u) += diagonal(Ml)
//       )
//     )
//   ));
 
  static boost::proto::terminal< ElementSystemMatrix< boost::mpl::int_<2> > >::type const M = {};
  
  // Pressure system assembly
  m_pressure_assembly->create_component<ProtoAction>(name)->set_expression(elements_expression(ElementsT(),
    group
    (
      _A = _0,
      compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
      element_quadrature( _A(p,p) += transpose(nabla(p)) * nabla(p) ),
      m_p_lss->system_matrix += -lit(theta) * (lit(tau_ps) + lit(dt)) * _A
    )
  ));
  
  // Lumped mass matrix assembly
  m_mass_matrix_assembly->create_component<ProtoAction>(name)->set_expression(elements_expression(ElementsT(),
    group
    (
      _A = _0,
      element_quadrature( _A(u[_i], u[_i]) += transpose(N(u)) * N(u) ),
      lump(_A),
      m_auu_lss->system_rhs += diagonal(_A)
    )
  ));
  
  // Assembly of the velocity matrices
  m_velocity_assembly->create_component<ProtoAction>(name)->set_expression(elements_expression(ElementsT(),
    group
    (
      _A = _0, _T = _0, M = _0,
      compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
      element_quadrature
      (
        M(u[_i], u[_i]) += nu_eff * transpose(nabla(u)) * nabla(u),
        M(u[_i], u[_j]) += transpose((tau_bulk + 0.33333333333333*nu_eff)*nabla(u)[_i]) * nabla(u)[_j],
        _T(u[_i], u[_i]) += transpose(N(u) + tau_su*u_adv*nabla(u)) * N(u),
        _A(u[_i], u[_i]) += transpose(N(u) + tau_su*u_adv*nabla(u)) * u_adv*nabla(u),
        _A(u[_i], u[_j]) += transpose(0.5*u_adv[_i]*(N(u) + tau_su*u_adv*nabla(u))) * nabla(u)[_j]
        
      ),
      m_u_lss->system_matrix += _T + lit(theta) * lit(dt) * M,
      m_auu_lss->system_matrix += _A + M
    )
  ));
  
  // Assembly of velocity RHS
  m_inner_loop->get_child("URHSAssembly")->create_component<ProtoAction>(name)->set_expression(elements_expression(ElementsT(),
    group
    (
      _A(u,u) = _0, _a = _0,
      compute_tau(u, nu_eff, lit(tau_su)),
      element_quadrature
      (
        _a[u[_i]] += (transpose(tau_su*u_adv*nabla(u)) * nabla(u)[_i] - transpose(nabla(u)[_i]) * N(u)) * ((1. - lit(theta))*delta_p_sum - p_vec) // Aup
                   - (transpose(N(u) + tau_su*u_adv*nabla(u)) * N(u)) * transpose(lit(a)[_i]) // Tuu
      ),
      m_u_lss->system_rhs += _a
    )
  ));
  
  // Assembly of pressure RHS
  m_inner_loop->get_child("PRHSAssembly")->create_component<ProtoAction>(name)->set_expression(elements_expression(ElementsT(),
    group
    (
      _A(p,p) = _0, _a = _0,
      compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
      element_quadrature
      (
        _a[p] += (transpose(N(p) + tau_ps*u_adv*nabla(p)*0.5) * nabla(p)[_i] + tau_ps * transpose(nabla(p)[_i]) * u_adv*nabla(p)) * transpose(lit(u_vec)[_i] + lit(dt)*lit(delta_a)[_i]) // Apu
              +   tau_ps * transpose(nabla(p)[_i]) * N(p) * transpose(lit(a)[_i] + lit(delta_a)[_i]), // Tpu
        _A(p,p) += tau_ps * transpose(nabla(p)) * nabla(p)
      ),
      m_p_lss->system_rhs += _a + _A*p_vec
    )
  ));
  
  // Apply Aup to delta_p
  m_inner_loop->get_child("ApplyAup")->create_component<ProtoAction>(name)->set_expression(elements_expression(ElementsT(),
    group
    (
      _A(u,u) = _0, _a = _0,
      compute_tau(u, nu_eff, lit(tau_su)),
      element_quadrature
      (
        _a[u[_i]] += (transpose(tau_su*u_adv*nabla(u)) * nabla(u)[_i] - transpose(nabla(u)[_i]) * N(u)) * delta_p // Aup
      ),
      m_u_lss->system_rhs += lit(theta) * _a
    )
  ));
}

} // UFEM
} // cf3

#endif // cf3_UFEM_semi_implicit_PressureMatrixAssembly_hpp
