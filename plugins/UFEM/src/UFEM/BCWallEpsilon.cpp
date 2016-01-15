// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include <common/EventHandler.hpp>

#include "math/LSS/System.hpp"

#include "mesh/Region.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"

#include "BCWallEpsilon.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;
using boost::proto::lit;

common::ComponentBuilder < BCWallEpsilon, common::Action, LibUFEM > BCWallEpsilon_Builder;

BCWallEpsilon::BCWallEpsilon(const std::string& name) :
  ProtoAction(name),
  m_rhs(options().add("lss", Handle<math::LSS::System>()).pretty_name("LSS").description("The linear system for which the boundary condition is applied"))
{
  FieldVariable<0, ScalarField> k("k", "ke_k");
  FieldVariable<1, ScalarField> epsilon("epsilon", "ke_epsilon");
  FieldVariable<2, VectorField> u("Velocity", "navier_stokes_solution");

  PhysicsConstant nu("kinematic_viscosity");

  const auto u_tau = make_lambda([&](const Real k, const Real u)
  {
    const Real u_k = k >= 0. ? ::pow(m_c_mu, 0.25)*::sqrt(k) : 0.;
    const Real u_tau = std::max(u_k, u/m_yplus);
    return u_tau;
  });

  const auto pow4 = make_lambda([](const Real x)
  {
    return x*x*x*x;
  });

  const auto pow5 = make_lambda([](const Real x)
  {
    return x*x*x*x*x;
  });

  // Linearization parameter gamma = epsilon / k:
  const auto gamma = make_lambda([&](const Real k, const Real nu, const Real epsilon)
  {
    if(k>1e-30 && epsilon > 1e-30)
    {
      return epsilon/k;
    }

    return m_c_mu * std::max(k,0.) / (m_kappa*m_yplus*nu);
  });

  set_expression(elements_expression
  (
    boost::mpl::vector1<mesh::LagrangeP1::Line2D>(),
    m_rhs(epsilon) += integral<4>(transpose(N(epsilon))*
    (
      pow5(u_tau(k, _norm(u))) / (lit(m_sigma_epsilon) * nu * lit(m_yplus)) // Epsilon Neumann BC
      //+ lit(m_c_epsilon_1)*gamma(k, nu, epsilon) * pow4(u_tau(k, _norm(u)))/(lit(m_kappa)*lit(m_yplus)*nu) // Wall production term
    )*_norm(normal)
  )));
}

BCWallEpsilon::~BCWallEpsilon()
{
}

} // namespace UFEM

} // namespace cf3
