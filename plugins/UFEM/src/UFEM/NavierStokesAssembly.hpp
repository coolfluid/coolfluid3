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

#include "NavierStokesSpecializations.hpp"
#include "SUPG.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

using boost::proto::lit;

template<typename GenericElementsT, typename SpecializedElementsT>
void NavierStokes::set_assembly_expression(const std::string& action_name)
{
  // Get all the relevant types as the concatenation of the generic and specialized element types:
  typedef typename boost::mpl::copy< SpecializedElementsT, boost::mpl::back_inserter<GenericElementsT> >::type AllElementsT;

  // Proto function that applies expressions only to GenericElementsT
  static const typename boost::proto::terminal< RestrictToElementTypeTag<GenericElementsT> >::type for_generic_elements = {};
  // Proto function that applies expressions only to SpecializedElementsT
  static const typename boost::proto::terminal< RestrictToElementTypeTag<SpecializedElementsT> >::type for_specialized_elements = {};
  
  const Real theta = options().option("theta").value<Real>();
  if(theta < 0. || theta > 1.)
    throw SetupError(FromHere(), "Value " + to_str(theta) + " for theta option of " + uri().path() + " is outside of the valid range from 0 to 1.");

  // The actual matrix assembly
  m_assembly->add_component(create_proto_action
  (
    action_name,
    elements_expression
    (
      AllElementsT(),
      group
      (
        _A = _0, _T = _0,
        for_generic_elements
        (
          compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
          element_quadrature
          (
            _A(p    , u[_i]) += transpose(N(p) + tau_ps*u_adv*nabla(p)*0.5) * nabla(u)[_i] + tau_ps * transpose(nabla(p)[_i]) * u_adv*nabla(u), // Standard continuity + PSPG for advection
            _A(p    , p)     += tau_ps * transpose(nabla(p)) * nabla(p) / rho, // Continuity, PSPG
            _A(u[_i], u[_i]) += nu_eff * transpose(nabla(u)) * nabla(u) + transpose(N(u) + tau_su*u_adv*nabla(u)) * u_adv*nabla(u), // Diffusion + advection
            _A(u[_i], p)     += transpose(N(u) + tau_su*u_adv*nabla(u)) * nabla(p)[_i] / rho, // Pressure gradient (standard and SUPG)
            _A(u[_i], u[_j]) += transpose((tau_bulk + 0.33333333333333*nu_eff)*nabla(u)[_i] // Bulk viscosity and second viscosity effect
                                + 0.5*u_adv[_i]*(N(u) + tau_su*u_adv*nabla(u))) * nabla(u)[_j],  // skew symmetric part of advection (standard +SUPG)
            _T(p    , u[_i]) += tau_ps * transpose(nabla(p)[_i]) * N(u), // Time, PSPG
            _T(u[_i], u[_i]) += transpose(N(u) + tau_su*u_adv*nabla(u)) * N(u) // Time, standard and SUPG
          )
        ),
        for_specialized_elements(supg_specialized(p, u, u_adv, nu_eff, u_ref, rho, _A, _T)),
        system_matrix += invdt() * _T + theta * _A,
        system_rhs += -_A * _x
      )
    )
  ));
}

} // UFEM
} // cf3
