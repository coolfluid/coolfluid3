// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_NavierStokesExplicitAssembly_hpp
#define cf3_UFEM_NavierStokesExplicitAssembly_hpp

#include "NavierStokesExplicit.hpp"

#include "solver/actions/Iterate.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "SUPG.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

using boost::proto::lit;


template<typename ElementsT>
void NavierStokesExplicit::set_velocity_assembly_expression(const std::string& base_name)
{
  m_inner_loop->add_component(create_proto_action
  (
    base_name+"VelocityAssembly",
    elements_expression
    (
      ElementsT(),
      group
      (
        _a = _0, _T = _0,
        compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
        element_quadrature
        (
          _a[u[_i]] += (nu_eff * transpose(nabla(u)) * nabla(u) // Diffusion
                       + transpose(N(u) + tau_su*u_adv*nabla(u)) * u_adv*nabla(u)) * transpose(transpose(nodal_values(u))[_i]) // Advection
                    -  (transpose(nabla(u)[_i])*N(p) - tau_su*transpose(u_adv*nabla(u)) * nabla(p)[_i]) / rho * nodal_values(p) // Pressure gradient (standard and SUPG)
                    +  transpose(N(u) + tau_su*u_adv*nabla(u)) * N(u) * transpose(transpose(nodal_values(a))[_i]), // Time, standard and SUPG
          _a[u[_i]] += transpose(0.5*u_adv[_i]*(N(u) + tau_su*u_adv*nabla(u)) + (tau_bulk + 0.33333333333333*nu_eff)*nabla(u)[_i]) * nabla(u)[_j] * transpose(transpose(nodal_values(u))[_j]), // Skew symmetry for advection and bulk viscosity
          _T(u[_i], u[_i]) += transpose(N(u)) * N(u)
        ),
        lump(_T),
        M += rho*diagonal(_T),
        R += -lit(rho)*_a
      )
  )));
}

template<typename ElementsT>
void NavierStokesExplicit::set_velocity_implicit_assembly_expression(const std::string& base_name)
{
  m_velocity_lss->add_component(create_proto_action
  (
    base_name+"VelocityAssembly",
    elements_expression
    (
      ElementsT(),
      group
      (
        _a = _0, _T = _0, _A = _0,
        compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
        element_quadrature
        (
          _A(u[_i], u[_i]) += nu_eff * transpose(nabla(u)) * nabla(u), // Diffusion
          _a[u[_i]]        += (transpose(N(u) + tau_su*u_adv*nabla(u)) * u_adv*nabla(u)) * transpose(transpose(nodal_values(u))[_i]) // Advection
                           -  (transpose(nabla(u)[_i])*N(p) - tau_su*transpose(u_adv*nabla(u)) * nabla(p)[_i]) / rho * nodal_values(p) // Pressure gradient (standard and SUPG)
                           +  transpose(N(u) + tau_su*u_adv*nabla(u)) * N(u) * transpose(transpose(nodal_values(a))[_i]), // Time, standard and SUPG
          _a[u[_i]] += transpose(0.5*u_adv[_i]*(N(u) + tau_su*u_adv*nabla(u))) * nabla(u)[_j] * transpose(transpose(nodal_values(u))[_j]), // Skew symmetry for advection
          _A(u[_i], u[_j]) += transpose((tau_bulk + 0.33333333333333*nu_eff)*nabla(u)[_i]) * nabla(u)[_j], // Bulk viscosity and second viscosity
          _T(u[_i], u[_i]) += transpose(N(u)) * N(u)
        ),
        _a[u[_i]] += _A(u[_i], u[_j])*transpose(transpose(nodal_values(u))[_j] + lit(gamma_u)*lit(m_dt)*transpose(nodal_values(a))[_j]),
        m_velocity_lss->system_matrix += rho*(_T + lit(gamma_u)*lit(m_dt)*_A),
        m_velocity_lss->system_rhs += -lit(rho)*(_a)
      )
  )));
}

template<typename ElementsT>
void NavierStokesExplicit::set_pressure_assembly_expression(const std::string& base_name)
{
  m_pressure_lss->add_component(create_proto_action
  (
    base_name+"PressureAssembly",
    elements_expression
    (
      ElementsT(),
      group
      (
        _a = _0, _T(p,p) = _0,
        compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
        element_quadrature
        (
          _a[p]         += tau_ps * transpose(nabla(p)[_i]) * N(u) * transpose(transpose(nodal_values(a))[_i] + transpose(nodal_values(delta_a_star))[_i]) // PSPG, time part
                        +  transpose(N(p)) * nabla(u)[_i] * transpose(transpose(nodal_values(u))[_i] + lit(gamma_u)*lit(m_dt)*(transpose(nodal_values(delta_a_star))[_i] + transpose(nodal_values(a))[_i])) // G'u + gamma_u dt G'Delta_a*
                        +  tau_ps * (transpose(nabla(p)[_i]) * u_adv*nabla(u) + transpose(u_adv*nabla(p)*0.5) * nabla(u)[_i]) * transpose(transpose(nodal_values(u))[_i]), // Standard + Skew symmetric advection PSPG term
          _T(p,p)       += transpose(nabla(p)) * nabla(p) // Pressure PSPG
        ),
        m_pressure_lss->system_matrix += (lit(tau_ps)/rho + rho*(lit(gamma_u)*lit(m_dt) + lit(tau_ps)))*_T,
        m_pressure_lss->system_rhs += -_a - tau_ps * _T * nodal_values(p) / rho
      )
  )));
}

template<typename ElementsT, typename RHST>
void NavierStokesExplicit::set_pressure_gradient_assembly_expression(const std::string& base_name, RHST& rhs)
{
  m_inner_loop->add_component(create_proto_action(base_name + "ApplyGrad", elements_expression
  (
    ElementsT(),
    group
    (
      _a[u] = _0, _A(u) = _0,
      compute_tau(u, nu_eff, lit(tau_su)),
      element_quadrature
      (
        _a[u[_i]] += transpose(nabla(u)[_i]) * N(p) * nodal_values(delta_p)
      ),
      rhs += _a
    )
  )));
}

} // UFEM
} // cf3

#endif // cf3_UFEM_NavierStokesExplicitAssembly_hpp

