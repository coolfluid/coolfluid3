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
void NavierStokesSemiImplicit::set_pressure_matrix_assembly(LSSActionUnsteady& lss, const std::string& action_name)
{  
  const Real theta = options().option("theta").value<Real>();
  if(theta < 0. || theta > 1.)
    throw SetupError(FromHere(), "Value " + to_str(theta) + " for theta option of " + uri().path() + " is outside of the valid range from 0 to 1.");

  lss.add_component(create_proto_action
  (
    action_name,
    elements_expression
    (
      ElementsT(),
      group
      (
        _A = _0, _T(u) = _0, _T(p) = _0,
        group
        (
          compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
          element_quadrature
          (
            _A(p    , u[_i]) += lit(lss.dt()) * (transpose(N(p) + tau_ps*u_adv*nabla(p)*0.5) * nabla(u)[_i] + tau_ps * transpose(nabla(p)[_i]) * u_adv*nabla(u)) + tau_ps * transpose(nabla(p)[_i]) * N(u), // dt*Apu + Tpu
            _A(p    , p)     += -lit(tau_ps) * transpose(nabla(p)) * nabla(p), // The minus accounts for the fact that we will subtract this part from the pressure matrix
            _A(u[_i], p)     += transpose(N(u) + tau_su*u_adv*nabla(u)) * nabla(p)[_i],
  _T(u[_i], u[_i]) += transpose(N(u) + tau_su*u_adv*nabla(u)) * N(u) + lit(theta)*lit(lss.dt())*(nu_eff * transpose(nabla(u)) * nabla(u) + transpose(N(u) + tau_su*u_adv*nabla(u)) * u_adv*nabla(u)),
  _T(u[_i], u[_j]) += lit(theta)*lit(lss.dt())*(transpose((tau_bulk + 0.33333333333333*nu_eff)*nabla(u)[_i] // Bulk viscosity and second viscosity effect
  + 0.5*u_adv[_i]*(N(u) + tau_su*u_adv*nabla(u))) * nabla(u)[_j])

          )
        ),
        lump(_T),
        lss.system_matrix += _T + _A
      )
    )
  ));
}

} // UFEM
} // cf3

#endif // cf3_UFEM_semi_implicit_PressureMatrixAssembly_hpp
