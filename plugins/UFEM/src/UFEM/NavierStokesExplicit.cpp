// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "NavierStokesExplicit.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <boost/mpl/back_inserter.hpp>
#include <boost/mpl/copy.hpp>

#include "common/Component.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/PropertyList.hpp"

#include "solver/actions/Iterate.hpp"
#include "solver/actions/SolveLSS.hpp"
#include "solver/actions/ZeroLSS.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/actions/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "NavierStokesSpecializations.hpp"
#include "ParsedFunctionExpression.hpp"
#include "SUPG.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

using boost::proto::lit;

ComponentBuilder < NavierStokesExplicit, LSSActionUnsteady, LibUFEM > NavierStokesExplicit_builder;

NavierStokesExplicit::NavierStokesExplicit(const std::string& name) :
  LSSActionUnsteady(name),
  u("Velocity", "navier_stokes_u_solution"),
  p("Pressure", "navier_stokes_p_solution"),
  a("a", "navier_stokes_explicit_iteration"),
  R("R", "navier_stokes_explicit_iteration"),
  M("M", "navier_stokes_explicit_iteration"),
  nu_eff("EffectiveViscosity", "navier_stokes_viscosity"),
  p_dot("p_dot", "navier_stokes_explicit_iteration"),
  u_star("u_star", "navier_stokes_explicit_iteration"),
  delta_a_star("delta_a_star", "navier_stokes_explicit_iteration"),
  delta_a("delta_a", "navier_stokes_explicit_iteration"),
  delta_p("delta_p", ",navier_stokes_explicit_iteration"),
  u_ref("reference_velocity"),
  rho("density"),
  nu("kinematic_viscosity"),
  gamma_u(0.5),
  gamma_p(0.5)
{
  options().add("use_specializations", true)
    .pretty_name("Use Specializations")
    .description("Activate the use of specialized high performance code")
    .attach_trigger(boost::bind(&NavierStokesExplicit::trigger_assembly, this));

  options().add("gamma_u", gamma_u)
    .pretty_name("Gamma U")
    .description("Velocity update parameter")
    .link_to(&gamma_u);

  options().add("gamma_p", gamma_p)
    .pretty_name("Gamma P")
    .description("Pressure update parameter")
    .link_to(&gamma_p);

  // The LSS is associated only with the pressure part
  set_solution_tag("navier_stokes_p_solution");

  // Initialization for the inner loop
  add_component(create_proto_action("InitializeIteration", nodes_expression(group
  (
    u = u + dt()*(1. - lit(gamma_u))*a,
    a[_i] = 0.,
    p = p + dt()*(1. - lit(gamma_p))*p_dot,
    p_dot = 0.
  ))));

  // Inner loop, executed several times per timestep
  m_inner_loop = create_component<solver::actions::Iterate>("InnerLoop");
  m_inner_loop->mark_basic();
  m_inner_loop->options().set("max_iter", 2);

  trigger_assembly();
}

NavierStokesExplicit::~NavierStokesExplicit()
{
}

void NavierStokesExplicit::trigger_assembly()
{
  m_inner_loop->clear();

  m_inner_loop->add_component(create_proto_action("ZeroLumpedSystem", nodes_expression(group(M[_i] = 0., R[_i] = 0.))));

  // Apply boundary conditions to the explicit "system". These are applied first to make sure the nodal values are correct for assembly
  Handle<BoundaryConditions> bc_u = m_inner_loop->create_component<BoundaryConditions>("VelocityBC");
  bc_u->mark_basic();
  bc_u->set_solution_tag("navier_stokes_u_solution");

  // First assemble the explicit momentum equation
  //set_triag_u_assembly();
  set_quad_u_assembly();

  m_inner_loop->add_link(*bc_u); // Make sure the system is updated to reflect the BC

  // Save the last residual
  FieldVariable<0, VectorField> saved_R("residual", "ns_residual");
  m_inner_loop->add_component(create_proto_action("SaveResidual", nodes_expression(saved_R = R)));

  // Update variables needed for the pressure system
  m_inner_loop->add_component(create_proto_action("SetPressureInput", nodes_expression(group
  (
    delta_a_star[_i] = R[_i]/M[_i],
    //u_star = u + lit(gamma_u)*dt()*delta_a_star,
    R[_i] = 0. // We reuse the residual vector later on, so reset it to zero
  ))));

  // Set the pressure LSS to zero
  m_inner_loop->create_component<ZeroLSS>("ZeroLSS");

  // Assembly of the pressure LSS
  //set_triag_p_assembly();
  set_quad_p_assembly();

  // Pressure BC
  Handle<BoundaryConditions> bc_p = m_inner_loop->create_component<BoundaryConditions>("PressureBC");
  bc_p->mark_basic();
  bc_p->set_solution_tag(solution_tag());

  // Solution of the system
  m_inner_loop->create_component<SolveLSS>("SolvePressureLSS");

  // Update deltap
  m_inner_loop->add_component(create_proto_action("SetDeltaP", nodes_expression(delta_p = solution(p))));

  // Apply the pressure gradient, storing the result in no longer needed R
  m_inner_loop->add_component(create_proto_action("ApplyGrad", elements_expression
  (
    boost::mpl::vector1<mesh::LagrangeP1::Quad2D>(),
    group
    (
      _a[u] = _0, _A(u) = _0,
      compute_tau(u, nu_eff, lit(tau_su)),
      element_quadrature
      (
        _a[u[_i]] += transpose(nabla(u)[_i]) * N(p) * nodal_values(delta_p)
      ),
      R += _a
    )
  )));

  // Update the rest of the variables
  m_inner_loop->add_component(create_proto_action("Update", nodes_expression(group
  (
    delta_a[_i] = delta_a_star[_i] + R[_i] / M[_i],
    u += gamma_u*lit(dt())*delta_a,
    a += delta_a,
    p += delta_p,
    p_dot += invdt() * delta_p / gamma_p
  ))));

  m_inner_loop->add_link(*bc_u);

  if(is_not_null(m_physical_model))
    configure_option_recursively(solver::Tags::physical_model(), m_physical_model);

  configure_option_recursively(solver::Tags::regions(), options().option(solver::Tags::regions()).value());
}

void NavierStokesExplicit::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  // Initial condition for the viscosity, defaulting to the molecular viscosity
  Handle<ProtoAction> visc_ic(initial_conditions.create_initial_condition("navier_stokes_viscosity", "cf3.solver.ProtoAction"));
  visc_ic->set_expression(nodes_expression(nu_eff = nu));

  initial_conditions.create_initial_condition(solution_tag());
  initial_conditions.create_initial_condition("navier_stokes_u_solution");
  initial_conditions.create_initial_condition("navier_stokes_explicit_iteration");
}

struct NavierStokesExplicitVelocityBC : ParsedFunctionExpression
{
  NavierStokesExplicitVelocityBC(const std::string& name) : ParsedFunctionExpression(name)
  {
    FieldVariable<0, VectorField> u("Velocity", "navier_stokes_u_solution");
    FieldVariable<1, VectorField> R("R", "navier_stokes_explicit_iteration");
    FieldVariable<2, VectorField> M("M", "navier_stokes_explicit_iteration");
    FieldVariable<3, VectorField> a("a", "navier_stokes_explicit_iteration");

    set_expression(nodes_expression(group
    (
      u = vector_function(),
      R[_i] = 0.,
      M[_i] = 1.,
      a[_i] = 0.
    )));
  }

  static std::string type_name () { return "NavierStokesExplicitVelocityBC"; }
};

common::ComponentBuilder < NavierStokesExplicitVelocityBC, common::Action, LibUFEM > NavierStokesExplicitVelocityBC_Builder;

} // UFEM
} // cf3
