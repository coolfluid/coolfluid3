// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "NavierStokes.hpp"

#include <boost/mpl/back_inserter.hpp>
#include <boost/mpl/copy.hpp>

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

namespace cf3 {
namespace UFEM {
namespace compressible {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

using boost::proto::lit;

template<typename ElementsT>
void NavierStokes::set_assembly_expression(const std::string& action_name)
{
  const Real theta = options().option("theta").value<Real>();
  if(theta < 0. || theta > 1.)
    throw SetupError(FromHere(), "Value " + to_str(theta) + " for theta option of " + uri().path() + " is outside of the valid range from 0 to 1.");

  // The actual matrix assembly
  m_assembly->add_component(create_proto_action
  (
    action_name,
    elements_expression
    (
      ElementsT(),
      group
      (
        _A = _0, _T = _0, _a = _0,
        compute_tau.apply(u_adv, nu_eff, lit(dt()), lit(tau_ps), lit(tau_su), lit(tau_bulk)),
        element_quadrature
        (
          _A(p    , u[_i]) += transpose(N(p)) * nabla(u)[_i] + tau_ps * transpose(nabla(p)[_i]) * u_adv*nabla(u), // Standard continuity + PSPG for advection
          _A(p    , p)     += tau_ps * transpose(nabla(p)) * nabla(p), // Continuity, PSPG
          _A(u[_i], u[_i]) += nu_eff * transpose(nabla(u)) * nabla(u) + transpose(N(u) + tau_su*u_adv*nabla(u)) * u_adv*nabla(u), // Diffusion + advection
          _A(u[_i], p)     += transpose(N(u) + tau_su*u_adv*nabla(u)) * nabla(p)[_i], // Pressure gradient (standard and SUPG)
          _A(u[_i], u[_j]) += transpose(tau_bulk*nabla(u)[_i]) * nabla(u)[_j], // Bulk viscosity
          _T(p    , u[_i]) += tau_ps * transpose(nabla(p)[_i]) * N(u), // Time, PSPG
          _T(u[_i], u[_i]) += transpose(N(u) + tau_su*u_adv*nabla(u)) * N(u), // Time, standard and SUPG
          _a[u[_i]] += transpose(N(u) + tau_su*u_adv*nabla(u)) * g[_i] * density_ratio,
          _a[p] += tau_ps * transpose(nabla(p)) * transpose(g) * density_ratio
        ),
        system_rhs += -_A * _x + _a,
        _A(p) = _A(p) / theta,
        system_matrix += invdt() * _T + theta * _A
      )
    )
  ));
}

} //compressible
} // UFEM
} // cf3
