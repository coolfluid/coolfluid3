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
void NavierStokesSemiImplicit::set_matrix_assembly(LSSAction& rhs_lss, LSSAction& t_lss, const std::string& action_name)
{  
  add_component(create_proto_action
  (
    action_name,
    elements_expression
    (
      ElementsT(),
      group
      (
        _A = _0, _T = _0,
        group
        (
          compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
          element_quadrature
          (
            _A(p    , u[_i]) += transpose(N(p) + tau_ps*u_adv*nabla(p)*0.5) * nabla(u)[_i] + tau_ps * transpose(nabla(p)[_i]) * u_adv*nabla(u), // Standard continuity + PSPG for advection
            _A(p    , p)     += tau_ps * transpose(nabla(p)) * nabla(p), // Continuity, PSPG
            _A(u[_i], u[_i]) += nu_eff * transpose(nabla(u)) * nabla(u) + transpose(N(u) + tau_su*u_adv*nabla(u)) * u_adv*nabla(u), // Diffusion + advection
            _A(u[_i], p)     += transpose(N(u) + tau_su*u_adv*nabla(u)) * nabla(p)[_i], // Pressure gradient (standard and SUPG)
            _A(u[_i], u[_j]) += transpose((tau_bulk + 0.33333333333333*nu_eff)*nabla(u)[_i] // Bulk viscosity and second viscosity effect
                                + 0.5*u_adv[_i]*(N(u) + tau_su*u_adv*nabla(u))) * nabla(u)[_j],  // skew symmetric part of advection (standard +SUPG)
            _T(p    , u[_i]) += tau_ps * transpose(nabla(p)[_i]) * N(u), // Time, PSPG
            _T(u[_i], u[_i]) += transpose(N(u) + tau_su*u_adv*nabla(u)) * N(u) // Time, standard and SUPG
          )
        ),
        rhs_lss.system_matrix += _A,
        t_lss.system_matrix += _T,
        _A(p) = _A(p) / theta,
        system_matrix += _T + lit(theta) * lit(dt()) * _A
      )
    )
  ));
}

} // UFEM
} // cf3

#endif // cf3_UFEM_semi_implicit_PressureMatrixAssembly_hpp
