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

#include "BCWallEpsilonABL.hpp"
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

common::ComponentBuilder < BCWallEpsilonABL, common::Action, LibUFEM > BCWallEpsilonABL_Builder;

BCWallEpsilonABL::BCWallEpsilonABL(const std::string& name) :
  ProtoAction(name),
  rhs(options().add("lss", Handle<math::LSS::System>()).pretty_name("LSS").description("The linear system for which the boundary condition is applied")),
  system_matrix(options().option("lss")),
  dirichlet(options().option("lss"))
{
  options().add("theta", m_theta)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .link_to(&m_theta);

  FieldVariable<0, ScalarField> k("k", "ke_solution");
  FieldVariable<1, ScalarField> epsilon("epsilon", "ke_solution");
  FieldVariable<2, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");

  PhysicsConstant zwall("zwall");
  PhysicsConstant z0("z0");
  PhysicsConstant nu_lam("kinematic_viscosity");

  set_expression(elements_expression
  (
    boost::mpl::vector3<mesh::LagrangeP1::Line2D, mesh::LagrangeP1::Triag3D, mesh::LagrangeP1::Quad3D>(), // Valid for surface element types
    group
    (
      _A(k) = _0, _A(epsilon) = _0,
      element_quadrature
      (
        _A(epsilon,epsilon) += -(nu_eff-nu_lam) / lit(m_sigma_epsilon) * transpose(N(epsilon)) * N(epsilon) * _norm(normal) / (lit(zwall) + lit(z0))
      ),
      system_matrix +=  m_theta * _A,
      rhs += -_A * _x
    )
  ));
}

BCWallEpsilonABL::~BCWallEpsilonABL()
{
}

} // namespace UFEM

} // namespace cf3
