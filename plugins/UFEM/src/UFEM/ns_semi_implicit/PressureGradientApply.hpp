// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_semi_implicit_PressureGradientApply_hpp
#define cf3_UFEM_semi_implicit_PressureGradientApply_hpp

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
void NavierStokesSemiImplicit::set_pressure_gradient_apply(LSSActionUnsteady& lss, const std::string& action_name)
{
  const Real theta = options().option("theta").value<Real>();
  if(theta < 0. || theta > 1.)
    throw SetupError(FromHere(), "Value " + to_str(theta) + " for theta option of " + uri().path() + " is outside of the valid range from 0 to 1.");

  m_inner_loop->add_component(create_proto_action(action_name + "ApplyGrad", elements_expression
  (
    ElementsT(),
    group
    (
      _a[a] = _0, _A(a) = _0,
      compute_tau(u, nu_eff, lit(tau_su)),
      element_quadrature
      (
        _a[a[_i]] += lit(theta)*(-transpose(nabla(u)[_i])*N(p) + tau_su*transpose(u_adv*nabla(u)) * nabla(p)[_i]) * nodal_values(dp)
      ),
      lss.system_rhs += _a
    )
  )));

}

} // UFEM
} // cf3

#endif // cf3_UFEM_semi_implicit_PressureGradientApply_hpp
