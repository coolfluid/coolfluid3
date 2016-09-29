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
  rhs(options().add("lss", Handle<math::LSS::System>()).pretty_name("LSS").description("The linear system for which the boundary condition is applied")),
  system_matrix(options().option("lss")),
  dirichlet(options().option("lss"))
{
  options().add("theta", m_theta)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .link_to(&m_theta);

  link_physics_constant("kappa", m_kappa);
  link_physics_constant("c_mu", m_c_mu);
  link_physics_constant("yplus", m_yplus);

  FieldVariable<0, ScalarField> k("k", "ke_solution");
  FieldVariable<1, ScalarField> epsilon("epsilon", "ke_solution");
  FieldVariable<2, VectorField> u("Velocity", "navier_stokes_solution");

  PhysicsConstant nu("kinematic_viscosity");

  const auto u_tau = make_lambda([&](const Real k, const Real u)
  {
    const Real ut1 = u / m_yplus;
    if(k <= 0.)
    {
      return ut1;
    }

    return std::max(::pow(m_c_mu, 0.25)*::sqrt(k), ut1);
  });

  const auto pow4 = make_lambda([](const Real x) { return x*x*x*x; });

  set_expression(elements_expression
  (
    boost::mpl::vector3<mesh::LagrangeP1::Line2D, mesh::LagrangeP1::Triag3D, mesh::LagrangeP1::Quad3D>(), // Valid for surface element types
    group
    (
      _A(k) = _0, _A(epsilon) = _0,
      element_quadrature
      (
        _A(epsilon,epsilon) += -transpose(N(epsilon))*N(epsilon)*m_kappa*u_tau(k, _norm(u))/lit(m_sigma_epsilon) * _norm(normal)
      ),
      system_matrix +=  m_theta * _A,
      rhs += -_A * _x
    )
  ));
}

BCWallEpsilon::~BCWallEpsilon()
{
}

} // namespace UFEM

} // namespace cf3
