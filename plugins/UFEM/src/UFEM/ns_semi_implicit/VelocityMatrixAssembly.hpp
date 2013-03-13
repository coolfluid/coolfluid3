// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_semi_implicit_VelocityMatrixAssembly_hpp
#define cf3_UFEM_semi_implicit_VelocityMatrixAssembly_hpp

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
void NavierStokesSemiImplicit::set_velocity_matrix_assembly(LSSActionUnsteady& lss, const std::string& action_name)
{
  const Real theta = options().option("theta").value<Real>();
  if(theta < 0. || theta > 1.)
    throw SetupError(FromHere(), "Value " + to_str(theta) + " for theta option of " + uri().path() + " is outside of the valid range from 0 to 1.");

  /// Implicit part of velocity element matrix
  static boost::proto::terminal< ElementSystemMatrix< boost::mpl::int_<2> > >::type const Ai = {};
  /// Explicit part of velocity element stiffness matrix
  static boost::proto::terminal< ElementSystemMatrix< boost::mpl::int_<3> > >::type const Ae = {};
  lss.add_component(create_proto_action
  (
    action_name+"VelocityAssembly",
    elements_expression
    (
      ElementsT(),
      group
      (
        group(_a = _0, _T = _0, _A = _0, Ai = _0, Ae = _0),
        compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
        element_quadrature
        (
          Ai(a[_i], a[_i]) += nu_eff * transpose(nabla(u)) * nabla(u), // Diffusion
          Ae(a[_i], a[_i]) += transpose(N(u) + tau_su*u_adv*nabla(u)) * u_adv*nabla(u), // advection
          Ai(a[_i], a[_j]) += transpose((tau_bulk + 0.33333333333333*nu_eff)*nabla(u)[_i]) * nabla(u)[_j], // Bulk viscosity and second viscosity effect
          Ae(a[_i], a[_j]) += transpose(0.5*u_adv[_i]*(N(u) + tau_su*u_adv*nabla(u))) * nabla(u)[_j],  // skew symmetric part of advection (standard +SUPG)
          _a[a[_i]]        += (transpose(nabla(u)[_i])*N(p) - transpose(tau_su*u_adv*nabla(u)) * nabla(p)[_i]) * nodal_values(p),
          _T(a[_i], a[_i]) += transpose(N(u) + tau_su*u_adv*nabla(u)) * N(u) // Time, standard and SUPG
        ),
        lss.system_matrix += _T + lit(theta)*lit(lss.dt())*(Ai+Ae),
        lss.system_rhs += _a - (Ae+Ai)*element_vector(u) - (_T + lit(theta)*lit(lss.dt())*(Ai+Ae))*element_vector(a)
      )
  )));
}

} // UFEM
} // cf3

#endif // cf3_UFEM_semi_implicit_VelocityMatrixAssembly_hpp
