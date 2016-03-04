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

#include "BCWallFunctionNSImplicit.hpp"
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

common::ComponentBuilder < BCWallFunctionNSImplicit, common::Action, LibUFEM > BCWallFunctionNSImplicit_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

using boost::proto::lit;

BCWallFunctionNSImplicit::BCWallFunctionNSImplicit(const std::string& name) :
  Action(name),
  rhs(options().add("lss", Handle<math::LSS::System>())
    .pretty_name("LSS")
    .description("The linear system for which the boundary condition is applied")),
  system_matrix(options().option("lss"))
{
  options().add("theta", m_theta)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .link_to(&m_theta);


  create_static_component<ProtoAction>("SetWallViscosity")->options().option("regions").add_tag("norecurse");
  create_static_component<ProtoAction>("WallLaw")->options().option("regions").add_tag("norecurse");

  link_physics_constant("c_mu", m_c_mu);
  link_physics_constant("yplus", m_yplus);

  trigger_setup();
}

BCWallFunctionNSImplicit::~BCWallFunctionNSImplicit()
{
}


void BCWallFunctionNSImplicit::on_regions_set()
{
  get_child("WallLaw")->options().set("regions", options()["regions"].value());
  get_child("SetWallViscosity")->options().set("regions", options()["regions"].value());
}

void BCWallFunctionNSImplicit::trigger_setup()
{
  Handle<ProtoAction> wall_law(get_child("WallLaw"));
  Handle<ProtoAction> set_wall_viscosity(get_child("SetWallViscosity"));

  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  FieldVariable<1, ScalarField> p("Pressure", "navier_stokes_solution");
  FieldVariable<2, ScalarField> k("k", "ke_solution");
  FieldVariable<3, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");

  PhysicsConstant nu_lam("kinematic_viscosity");
  PhysicsConstant kappa("kappa");

  const auto tau_w = make_lambda([&](const Real k, const Real u)
  {
    if(k <= 0.)
    {
      return u / (m_yplus*m_yplus);
    }

    return ::pow(m_c_mu, 0.25)*::sqrt(k)/m_yplus;
  });

  set_wall_viscosity->set_expression(nodes_expression
  (
    nu_eff = lit(nu_lam) + lit(nu_lam)*lit(kappa)*lit(m_yplus)
  ));

  // Set normal component to zero and tangential component to the wall-law value
  wall_law->set_expression(elements_expression
  (
    boost::mpl::vector1<mesh::LagrangeP1::Line2D>(), // Valid for surface element types
    group
    (
      _A(u) = _0, _A(p) = _0,
      element_quadrature
      (
        _A(p, u[_i]) += -transpose(N(p)) * N(u) * normal[_i], // no-penetration condition
        _A(u[_i], u[_i]) += transpose(N(u)) * tau_w(k, _norm(u)) * N(u) * _norm(normal)
      ),
      system_matrix += m_theta * _A,
      rhs += -_A * _x
    )
  ));


}

void BCWallFunctionNSImplicit::execute()
{
  Handle<ProtoAction> set_wall_viscosity(get_child("SetWallViscosity"));
  Handle<ProtoAction> wall_law(get_child("WallLaw"));

  set_wall_viscosity->execute();
  wall_law->execute();
}

} // namespace UFEM

} // namespace cf3
