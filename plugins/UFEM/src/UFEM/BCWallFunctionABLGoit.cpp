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

#include "BCWallFunctionABLGoit.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "solver/actions/Proto/ElementGradDiv.hpp"
#include "solver/actions/Proto/Partial.hpp"
#include <vector>
#include <iostream>
#include <cmath>
#include <math.h>

namespace cf3
{

namespace UFEM
{
using namespace solver::actions::Proto;
using boost::proto::lit;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < BCWallFunctionABLGoit, common::Action, LibUFEM > BCWallFunctionABLGoit_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

BCWallFunctionABLGoit::BCWallFunctionABLGoit(const std::string& name) :
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
  link_physics_constant("kappa", m_kappa);
  link_physics_constant("zwall", m_zwall);
  link_physics_constant("z0", m_z0);

  trigger_setup();
}

BCWallFunctionABLGoit::~BCWallFunctionABLGoit()
{
}


void BCWallFunctionABLGoit::on_regions_set()
{
  get_child("WallLaw")->options().set("regions", options()["regions"].value());
  get_child("SetWallViscosity")->options().set("regions", options()["regions"].value());
}

void BCWallFunctionABLGoit::trigger_setup()
{
  Handle<ProtoAction> wall_law(get_child("WallLaw"));
  Handle<ProtoAction> set_wall_viscosity(get_child("SetWallViscosity"));

  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_u_solution");
  // FieldVariable<1, ScalarField> p("Pressure", "navier_stokes_solution");
  FieldVariable<2, ScalarField> k("k", "ke_solution");
  FieldVariable<3, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");

  PhysicsConstant nu_lam("kinematic_viscosity");

  const auto u_tau = make_lambda([&](const Real k, const Real u)
  {
    const Real ut1 = u / m_uplus;
    if(k <= 0.)
    {
      return ut1;
    }

    return std::max(::pow(m_c_mu, 0.25)*::sqrt(k), ut1);
  });

  const auto ABL_factor = make_lambda([&](const Real kappa, const Real zwall, const Real z0, const Real nu_eff)
  {
    Real factor = (::pow(kappa/::log(zwall/z0),2))/nu_eff;
    // std::cout << "yop; kappa: " << kappa << "; zwall: " << zwall << "; z0: " << z0 << "; nu_eff: " << nu_eff << std::endl;
    std::cout << "abl factor: " << factor << std::endl;
    return factor;
  });

  set_wall_viscosity->set_expression(nodes_expression
  (
    nu_eff = lit(nu_lam) + lit(m_kappa)*u_tau(k, _norm(u))*lit(m_zwall)
  ));

  // Set normal component to zero and tangential component to the wall-law value
  wall_law->set_expression(elements_expression
  (
    boost::mpl::vector3<mesh::LagrangeP1::Line2D, mesh::LagrangeP1::Triag3D, mesh::LagrangeP1::Quad3D>(), // Valid for surface element types
    group
    (
      _A(u) = _0, // _A(p) = _0,
      element_quadrature
      (
        // _A(p, u[_i]) += -transpose(N(p)) * N(u) * normal[_i], // no-penetration condition
        _A(u[_i], u[_i]) += ABL_factor(m_kappa, m_zwall, m_z0, nu_eff) * _norm(u) * transpose(N(u)) * N(u) * _norm(normal) // Goit p. 19
      ),
      system_matrix += m_theta * _A,
      rhs += -_A * _x
    )
  ));
}

void BCWallFunctionABLGoit::execute()
{
  m_uplus = 1./m_kappa * std::log((m_zwall + m_z0)/m_z0);

  Handle<ProtoAction> set_wall_viscosity(get_child("SetWallViscosity"));
  Handle<ProtoAction> wall_law(get_child("WallLaw"));

  set_wall_viscosity->execute();
  wall_law->execute();
}

} // namespace UFEM

} // namespace cf3
