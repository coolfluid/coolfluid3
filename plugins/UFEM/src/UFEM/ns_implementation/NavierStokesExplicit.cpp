// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "../NavierStokesExplicit.hpp"

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
#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/actions/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "../InitialConditions.hpp"
#include "../ParsedFunctionExpression.hpp"
#include "../SUPG.hpp"
#include "../Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

using boost::proto::lit;

ComponentBuilder < NavierStokesExplicit, common::Action, LibUFEM > NavierStokesExplicit_builder;

NavierStokesExplicit::NavierStokesExplicit(const std::string& name) :
  solver::ActionDirector(name),
  u("Velocity", "navier_stokes_u_solution"),
  p("Pressure", "navier_stokes_p_solution"),
  a("a", "navier_stokes_explicit_iteration"),
  R("R", "navier_stokes_explicit_iteration"),
  M("M", "navier_stokes_explicit_iteration"),
  nu_eff("EffectiveViscosity", "navier_stokes_viscosity"),
  p_dot("p_dot", "navier_stokes_explicit_iteration"),
  delta_a_star("delta_a_star", "navier_stokes_explicit_iteration"),
  delta_a("delta_a", "navier_stokes_explicit_iteration"),
  delta_p("delta_p", "navier_stokes_explicit_iteration"),
  u_adv("AdvectionVelocity", "linearized_velocity"),
  u_ref("reference_velocity"),
  rho("density"),
  nu("kinematic_viscosity"),
  gamma_u(0.5),
  gamma_p(0.5),
  m_recursing(false)
{
  options().add("implicit_diffusion", false)
    .pretty_name("Implicit Diffusion")
    .description("Make the diffsive terms implicit, adding a large linear system for the velocity but increasing the allowed time step")
    .attach_trigger(boost::bind(&NavierStokesExplicit::trigger_assembly, this));

  options().add("gamma_u", gamma_u)
    .pretty_name("Gamma U")
    .description("Velocity update parameter")
    .link_to(&gamma_u);

  options().add("gamma_p", gamma_p)
    .pretty_name("Gamma P")
    .description("Pressure update parameter")
    .link_to(&gamma_p);

  options().add(solver::Tags::time(), Handle< solver::Time >())
    .pretty_name("Time")
    .description("Component that keeps track of time for this simulation")
    .attach_trigger(boost::bind(&NavierStokesExplicit::trigger_time, this));

  options().add("initial_conditions", Handle<UFEM::InitialConditions>())
    .pretty_name("Initial Conditions")
    .description("The component that is used to manage the initial conditions in the solver this action belongs to")
    .attach_trigger(boost::bind(&NavierStokesExplicit::trigger_initial_conditions, this));

  // CFL adjustment
  create_component("ComputeCFL", "cf3.UFEM.ComputeCFL");

  // Initialization for the inner loop
  add_component(create_proto_action("InitializeIteration", nodes_expression(group
  (
    u = u + m_dt*(1. - lit(gamma_u))*a,
    a[_i] = 0.,
    p = p + m_dt*(1. - lit(gamma_p))*p_dot,
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

void NavierStokesExplicit::execute()
{
  solver::ActionDirector::execute();
  std::vector<std::string> disabled(2);
  disabled[0] = "PressureBC";
  disabled[1] = "PressureMatrixAssembly";
  m_pressure_lss->options().set("disabled_actions", disabled);
}

void NavierStokesExplicit::trigger_assembly()
{
  const bool implicit_diffusion = options().option("implicit_diffusion").value<bool>();

  m_inner_loop->clear();

  m_inner_loop->add_component(create_proto_action("InnerLoopInit", nodes_expression(group(M[_i] = 0., R[_i] = 0., u_adv = u))));

  // Apply boundary conditions to the explicit "system". These are applied first to make sure the nodal values are correct for assembly
  boost::shared_ptr<BoundaryConditions> bc_u = allocate_component<BoundaryConditions>("VelocityBC");
  bc_u->mark_basic();
  bc_u->set_solution_tag("navier_stokes_u_solution");

  if(!implicit_diffusion)
  {
    m_inner_loop->add_component(bc_u);

    // First assemble the explicit momentum equation
    set_triag_u_assembly();
    set_quad_u_assembly();
    set_hexa_u_assembly();
    set_tetra_u_assembly();

    m_inner_loop->add_link(*bc_u); // Make sure the system is updated to reflect the BC

    // Save the last residual
    FieldVariable<0, VectorField> saved_R("residual", "ns_residual");
    m_inner_loop->add_component(create_proto_action("SaveResidual", nodes_expression(saved_R = R)));

    // Update variables needed for the pressure system
    m_inner_loop->add_component(create_proto_action("SetPressureInput", nodes_expression(group
    (
      delta_a_star[_i] = R[_i]/M[_i],
      R[_i] = 0. // We reuse the residual vector later on, so reset it to zero
    ))));
  }
  else // Create a linear system for the velocity
  {
    m_velocity_lss = m_inner_loop->create_component<LSSActionUnsteady>("VelocitySystem");
    m_velocity_lss->set_solution_tag("navier_stokes_u_solution");
    m_velocity_lss->mark_basic();

    m_velocity_lss->create_component<math::LSS::ZeroLSS>("ZeroLSS");

    m_velocity_lss->add_component(bc_u);

    set_quad_implicit_u_assembly();
    set_triag_implicit_u_assembly();
    set_hexa_implicit_u_assembly();
    set_tetra_implicit_u_assembly();

    m_velocity_lss->add_link(*bc_u);
    m_velocity_lss->create_component<math::LSS::SolveLSS>("SolveVelocityLSS");

    // Update variables needed for the pressure system
    m_velocity_lss->add_component(create_proto_action("SetPressureInput", nodes_expression
    (
      delta_a_star = m_velocity_lss->solution(u)
    )));

    Handle<Component> reset_rhs = m_velocity_lss->create_component<math::LSS::ZeroLSS>("ZeroVelocityRHS");
    reset_rhs->options().set("reset_matrix", false);
    reset_rhs->options().set("reset_solution", false);
  }

  m_pressure_lss = m_inner_loop->create_component<LSSActionUnsteady>("PressureSystem");
  m_pressure_lss->set_solution_tag("navier_stokes_p_solution");
  m_pressure_lss->mark_basic();

  m_pressure_matrix_assembly = m_pressure_lss->create_component<solver::ActionDirector>("PressureMatrixAssembly");
  
  // Assembly of the pressure LSS RHS
  set_triag_p_rhs_assembly();
  set_quad_p_rhs_assembly();
  set_hexa_p_rhs_assembly();
  set_tetra_p_rhs_assembly();
  
  // Assembly of the pressure LSS matrix
  set_triag_p_mat_assembly();
  set_quad_p_mat_assembly();
  set_hexa_p_mat_assembly();
  set_tetra_p_mat_assembly();

  // Pressure BC
  Handle<BoundaryConditions> bc_p = m_pressure_lss->create_component<BoundaryConditions>("PressureBC");
  bc_p->mark_basic();
  bc_p->set_solution_tag("navier_stokes_p_solution");

  // Solution of the system
  m_pressure_lss->create_component<math::LSS::SolveLSS>("SolvePressureLSS");

  // Update deltap
  m_inner_loop->add_component(create_proto_action("SetDeltaP", nodes_expression(delta_p = m_pressure_lss->solution(p))));

  // Apply the pressure gradient, storing the result in no longer needed R
  if(implicit_diffusion)
  {
    set_quad_grad_p_assembly(m_velocity_lss->system_rhs);
    set_triag_grad_p_assembly(m_velocity_lss->system_rhs);
    set_hexa_grad_p_assembly(m_velocity_lss->system_rhs);
    set_tetra_grad_p_assembly(m_velocity_lss->system_rhs);
  }
  else
  {
    set_quad_grad_p_assembly(R);
    set_triag_grad_p_assembly(R);
    set_hexa_grad_p_assembly(R);
    set_tetra_grad_p_assembly(R);
  }

  m_inner_loop->add_link(*bc_u);

  if(implicit_diffusion)
  {
    m_inner_loop->add_link(*m_velocity_lss->get_child("SolveVelocityLSS"));
    m_inner_loop->add_component(create_proto_action("UpdateDeltaA", nodes_expression(delta_a = delta_a_star + m_velocity_lss->solution(u))));
  }
  else
  {
    m_inner_loop->add_component(create_proto_action("UpdateDeltaA", nodes_expression(delta_a[_i] = delta_a_star[_i] + R[_i] / M[_i])));
  }

  // Update the rest of the variables
  m_inner_loop->add_component(create_proto_action("Update", nodes_expression(group
  (
    u += gamma_u*lit(m_dt)*delta_a,
    a += delta_a,
    p += delta_p,
    p_dot += m_inv_dt * delta_p / gamma_p
  ))));

  if(is_not_null(m_physical_model))
    configure_option_recursively(solver::Tags::physical_model(), m_physical_model);

  configure_option_recursively(solver::Tags::regions(), options().option(solver::Tags::regions()).value());
}

void NavierStokesExplicit::trigger_time()
{
  if(is_not_null(m_time))
    m_time->options().option("time_step").detach_trigger(m_time_trigger_id);

  m_time = options().option("time").value< Handle<solver::Time> >();
  m_time_trigger_id = m_time->options().option("time_step").attach_trigger_tracked(boost::bind(&NavierStokesExplicit::trigger_timestep, this));
  trigger_timestep();
}

void NavierStokesExplicit::trigger_timestep()
{
  cf3_assert(is_not_null(m_time));
  m_dt = m_time->dt();
  m_inv_dt = m_time->invdt();
  get_child("ComputeCFL")->options().set("time_step", m_dt);
  if(is_not_null(m_pressure_lss))
    m_pressure_lss->options().set("disabled_actions", std::vector<std::string>());
}


void NavierStokesExplicit::trigger_initial_conditions()
{
  Handle<UFEM::InitialConditions> ic = options().option("initial_conditions").value< Handle<UFEM::InitialConditions> >();
  if(is_not_null(ic))
    on_initial_conditions_set(*ic);
}


void NavierStokesExplicit::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  // Initial condition for the viscosity, defaulting to the molecular viscosity
  Handle<ProtoAction> visc_ic(initial_conditions.create_initial_condition("navier_stokes_viscosity", "cf3.solver.ProtoAction"));
  visc_ic->set_expression(nodes_expression(nu_eff = nu));
  m_viscosity_initial_condition = visc_ic;

  m_velocity_initial_condition = initial_conditions.create_initial_condition("navier_stokes_u_solution");
  m_pressure_initial_condition = initial_conditions.create_initial_condition("navier_stokes_p_solution");
  m_iteration_initial_condition = initial_conditions.create_initial_condition("navier_stokes_explicit_iteration");
}

void NavierStokesExplicit::on_regions_set()
{
  if(m_recursing)
    return;

  m_recursing = true;

  cf3_assert(is_not_null(m_pressure_lss));
  m_pressure_lss->options().set("disabled_actions", std::vector<std::string>());
  configure_option_recursively(solver::Tags::regions(), options().option(solver::Tags::regions()).value());

  if(is_not_null(m_viscosity_initial_condition))
    m_viscosity_initial_condition->options().set(solver::Tags::regions(), options().option(solver::Tags::regions()).value());
  if(is_not_null(m_velocity_initial_condition))
    m_velocity_initial_condition->options().set(solver::Tags::regions(), options().option(solver::Tags::regions()).value());
  if(is_not_null(m_pressure_initial_condition))
    m_pressure_initial_condition->options().set(solver::Tags::regions(), options().option(solver::Tags::regions()).value());
  if(is_not_null(m_iteration_initial_condition))
    m_iteration_initial_condition->options().set(solver::Tags::regions(), options().option(solver::Tags::regions()).value());

  cf3::solver::ActionDirector::on_regions_set();

  m_recursing = false;
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

struct NavierStokesSemiExplicitVelocityBC : ParsedFunctionExpression
{
  NavierStokesSemiExplicitVelocityBC(const std::string& name) :
    ParsedFunctionExpression(name),
    m_dirichlet(options().add("lss", Handle<math::LSS::System>()).pretty_name("LSS").description("The linear system for which the boundary condition is applied"))
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
      a[_i] = 0.,
      m_dirichlet(a) = a
    )));
  }

  static std::string type_name () { return "NavierStokesSemiExplicitVelocityBC"; }

  cf3::solver::actions::Proto::DirichletBC m_dirichlet;
};

common::ComponentBuilder < NavierStokesSemiExplicitVelocityBC, common::Action, LibUFEM > NavierStokesSemiExplicitVelocityBC_Builder;


} // UFEM
} // cf3
