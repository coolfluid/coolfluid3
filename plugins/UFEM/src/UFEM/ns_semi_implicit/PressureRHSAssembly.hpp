// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_semi_implicit_PressureRHSAssembly_hpp
#define cf3_UFEM_semi_implicit_PressureRHSAssembly_hpp

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
void NavierStokesSemiImplicit::set_pressure_rhs_assembly(LSSActionUnsteady& lss, const std::string& action_name)
{
  lss.add_component(create_proto_action
  (
    action_name+"PressureRHSAssembly",
    elements_expression
    (
      ElementsT(),
      group
      (
        _A(dp,dp) = _0, _a = _0,
        compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
       //lit(tau_ps) = 0.,
        element_quadrature
        (
          _a[dp] += tau_ps * transpose(nabla(p)[_i]) * N(u) * transpose(transpose(nodal_values(delta_a_star))[_i]) // PSPG, time part
                 +  (transpose(N(p) + tau_ps*u_adv*nabla(p)*0.5) * nabla(u)[_i] + tau_ps * transpose(nabla(p)[_i]) * u_adv*nabla(u)) * transpose(transpose(nodal_values(u))[_i] + lit(lss.dt())*(transpose(nodal_values(delta_a_star))[_i])) // RHS Apu contribution
                 +  lit(tau_ps) * transpose(nabla(p)) * nabla(p) * nodal_values(p)
        ),
        lss.system_rhs += _a
      )
  )));
}

} // UFEM
} // cf3

#endif // cf3_UFEM_semi_implicit_PressureRHSAssembly_hpp
