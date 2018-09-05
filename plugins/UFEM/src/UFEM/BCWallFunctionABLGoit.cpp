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

#include <iostream>
#include <cmath>

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


  create_static_component<ProtoAction>("WallLaw")->options().option("regions").add_tag("norecurse");

  link_physics_constant("c_mu", m_c_mu);
  link_physics_constant("kappa", m_kappa);
  link_physics_constant("zwall", m_zwall);
  link_physics_constant("z0", m_z0);
  // link_physics_constant("u_adv", u_adv);
  // link_physics_constant("dynamic_viscosity", m_mu);
  // link_physics_constant("uref", uref);
  // link_physics_constant("href", href);
  // std::cout << "yop: Goit: mu: " << m_mu << "; kappa: " << m_kappa << "; zwall: " << m_zwall << "; z0: " << m_z0 << "; href: " << href << "; uref: " << uref << std::endl;

  trigger_setup();
}

BCWallFunctionABLGoit::~BCWallFunctionABLGoit()
{
}


void BCWallFunctionABLGoit::on_regions_set()
{
  get_child("WallLaw")->options().set("regions", options()["regions"].value());
}

void BCWallFunctionABLGoit::trigger_setup()
{
  Handle<ProtoAction> wall_law(get_child("WallLaw"));
  
  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  FieldVariable<1, ScalarField> p("Pressure", "navier_stokes_solution");
  FieldVariable<2, ScalarField> k("k", "ke_solution");
  FieldVariable<3, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");
  FieldVariable<4, VectorField> u_adv("AdvectionVelocity", "linearized_velocity");

  const auto ABL_factor = make_lambda([&](const Real nu_eff)
  {
    // Real factor = ::pow(m_kappa/::log(m_zwall/m_z0),2)/nu_eff;
    Real factor = ::pow(m_kappa/::log(m_zwall/m_z0),2);
    // Real factor = 1;
    //std::cout << "yop: abl factor: " << factor << "; nu_eff: " << nu_eff << "; kappa: " << m_kappa << "; zwall: " << m_zwall << "; z0: " << m_z0 << "; theta: " << m_theta << std::endl;

    return factor;
  });

  // Set normal component to zero and tangential component to the wall-law value
  wall_law->set_expression(elements_expression
  (
    // boost::mpl::vector3<mesh::LagrangeP1::Line2D, mesh::LagrangeP1::Triag3D, mesh::LagrangeP1::Quad3D>(), // Valid for surface element types
    boost::mpl::vector<mesh::LagrangeP1::Line2D>(), //mesh::LagrangeP1::Triag3D, mesh::LagrangeP1::Quad3D>(), // Valid for surface element types
    group
    (
      _A(u) = _0, _A(p) = _0,
      element_quadrature
      (
        // _A(p, u[_i]) += -transpose(N(p)) * N(u) * normal[_i], // no-penetration condition
        _A(u[_i], u[_i]) += ABL_factor(nu_eff) * _norm(u) * transpose(N(u)) * N(u) * _norm(normal) // Goit p. 19 
        // _A(u[_i], u[_i]) +=  ABL_factor(nu_eff) * _norm(u) * transpose(N(u) + 0.437995*u_adv[0]*nabla(u)) * N(u) * _norm(normal) // N(u) + tau_su*u_adv*nabla(u)
      ),
      system_matrix += m_theta * _A,
      rhs += -_A * _x
      // _cout << _A << "\n"
    )
  ));
}

void BCWallFunctionABLGoit::execute()
{
  Handle<ProtoAction> wall_law(get_child("WallLaw"));

  wall_law->execute();
}

} // namespace UFEM

} // namespace cf3
