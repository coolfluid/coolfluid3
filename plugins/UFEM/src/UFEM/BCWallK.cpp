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

#include "BCWallK.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"


namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < BCWallK, common::Action, LibUFEM > BCWallK_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

using boost::proto::lit;

BCWallK::BCWallK(const std::string& name) :
  Action(name),
  rhs(options().add("lss", Handle<math::LSS::System>())
    .pretty_name("LSS")
    .description("The linear system for which the boundary condition is applied")),
  system_matrix(options().option("lss"))
{
  create_static_component<ProtoAction>("WallViscosity")->options().option("regions").add_tag("norecurse");
  create_static_component<ProtoAction>("WallProduction")->options().option("regions").add_tag("norecurse");

  trigger_setup();
}

BCWallK::~BCWallK()
{
}

void BCWallK::on_regions_set()
{

  get_child("WallViscosity")->options().set("regions", options()["regions"].value());
  get_child("WallProduction")->options().set("regions", options()["regions"].value());
}

void BCWallK::trigger_setup()
{
  Handle<ProtoAction> wall_viscosity(get_child("WallViscosity"));
  Handle<ProtoAction> wall_production(get_child("WallProduction"));

  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  FieldVariable<1, ScalarField> k("k", "ke_k");
  FieldVariable<2, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");

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

  // Set production at the wall
  wall_production->set_expression(elements_expression
  (
    boost::mpl::vector1<mesh::LagrangeP1::Line2D>(),
    rhs(k) += integral<4>(transpose(N(k))*pow4(u_tau(k, _norm(u)))/(lit(m_kappa)*lit(m_yplus)*nu)*_norm(normal))
  ));

  // wall_viscosity->set_expression(nodes_expression
  // (
  //   nu_eff = nu*(lit(1.) + lit(m_kappa)*lit(m_yplus))
  // ));

}

void BCWallK::execute()
{
  // Handle<ProtoAction> wall_viscosity(get_child("WallViscosity"));
  Handle<ProtoAction> wall_production(get_child("WallProduction"));

  // wall_viscosity->execute();
  wall_production->execute();
}

} // namespace UFEM

} // namespace cf3
