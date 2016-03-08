// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Adjoint.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <boost/mpl/back_inserter.hpp>
#include <boost/mpl/copy.hpp>

#include "common/Component.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/PropertyList.hpp"

#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "../NavierStokesSpecializations.hpp"
#include "../Tags.hpp"

namespace cf3 {
namespace UFEM {
namespace adjoint{

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;
using boost::proto::lit;

ComponentBuilder < Adjoint, LSSActionUnsteady, LibUFEMAdjoint > Adjoint_Builder;

Adjoint::Adjoint(const std::string& name) :
  LSSActionUnsteady(name),
  u("Velocity", "navier_stokes_solution"),
  p("Pressure", "navier_stokes_solution"),
  U("Velocity", "adjoint_solution"),
  q("Pressure", "adjoint_solution"),
  U_adv("AdvectionVelocity", "linearized_velocity"),
  U1("AdvectionVelocity1", "linearized_velocity"),
  U2("AdvectionVelocity2", "linearized_velocity"),
  //U3("AdvectionVelocity3", "linearized_velocity"),
  nu_eff("EffectiveViscosity", "navier_stokes_viscosity"),
  T("Temperature", "scalar_advection_solution"),
  g("Force", "body_force"),
  rho("density"),
  nu("kinematic_viscosity"),
  Tref("reference_temperature")
{
  const std::vector<std::string> restart_field_tags = boost::assign::list_of("navier_stokes_solution")("adjoint_solution")("linearized_velocity")("navier_stokes_viscosity");
  properties().add("restart_field_tags", restart_field_tags);


  options().add("supg_type", compute_tau.data.op.supg_type_str)
    .pretty_name("SUPG Type")
    .description("Type of computation for the stabilization coefficients.")
    .link_to(&(compute_tau.data.op.supg_type_str))
    .attach_trigger(boost::bind(&ComputeTauImpl::trigger_supg_type, &compute_tau.data.op));

  options().add("u_ref", compute_tau.data.op.u_ref)
    .pretty_name("Reference velocity")
    .description("Reference velocity for the CF2 SUPG method")
    .link_to(&(compute_tau.data.op.u_ref));

  options().add("theta", 1.)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .attach_trigger(boost::bind(&Adjoint::trigger_assembly, this));

  options().add("enable_body_force", false)
    .pretty_name("Enable Body Force")
    .description("Set to true to enable the body force term")
    .attach_trigger(boost::bind(&Adjoint::trigger_enable_body_force, this))
    .mark_basic();

  options().option("regions").add_tag("norecurse");

  set_solution_tag("adjoint_solution");

  // This ensures that the linear system matrix is reset to zero each timestep
  create_component<math::LSS::ZeroLSS>("ZeroLSS")->options().set("reset_solution", false);
  // Extrapolate the velocity
  //add_component(create_proto_action("LinearizeU", nodes_expression(u_adv = 2.1875*u - 2.1875*u1 + 1.3125*u2 - 0.3125*u3)));
  add_component(create_proto_action("LinearizeU", nodes_expression(U_adv = U)));

  // Container for the assembly actions. Will be filled depending on the value of options, such as using specializations or not
  m_assembly = create_component<solver::ActionDirector>("Assembly");

  // Boundary conditions
  Handle<BoundaryConditions> bc =  create_component<BoundaryConditions>("BoundaryConditions");
  bc->mark_basic();
  bc->set_solution_tag(solution_tag());

  // Solution of the LSS
  create_component<math::LSS::SolveLSS>("SolveLSS");

  // Update of the solution
  m_update = create_component<solver::ActionDirector>("UpdateActions");


  trigger_assembly();
}

Adjoint::~Adjoint()
{
}


void Adjoint::trigger_assembly()
{
  m_assembly->clear();
  m_update->clear();
  m_assembly->add_component(create_proto_action
                            (
                              "AdjointAssembly",
                              elements_expression
                              (
                                boost::mpl::vector1<
                                    mesh::LagrangeP1::Triag2D
                                    //mesh::LagrangeP1::Tetra3D
                                    >(),
                                group
                                (
                                  _A = _0, _T = _0, _a = _0,

                                    compute_tau.apply(U_adv, nu_eff, lit(dt()), lit(tau_ps), lit(tau_su), lit(tau_bulk)),
                                    element_quadrature
                                    (
                                            _A(q    , U[_i]) += transpose(N(q) + tau_ps*U_adv*nabla(q)*0.5) * nabla(U)[_i] + tau_ps * transpose(nabla(q)[_i]) * U_adv*nabla(U), // Standard continuity + PSPG for advection
                                            _A(q    , q)     += tau_ps * transpose(nabla(q)) * nabla(q), // Continuity, PSPG
                                            _A(U[_i], U[_i]) += nu_eff * transpose(nabla(U)) * nabla(U) + transpose(N(U) + tau_su*U_adv*nabla(U)) * U_adv*nabla(U), // Diffusion + advection
                                            _A(U[_i], q)     += transpose(N(U) + tau_su*U_adv*nabla(U)) * nabla(q)[_i], // Pressure gradient (standard and SUPG)
                                            _A(U[_i], U[_j]) += transpose(tau_bulk*nabla(U)[_i] // Bulk viscosity
                                                                + 0.5*U_adv[_i]*(N(U) + tau_su*U_adv*nabla(U))) * nabla(U)[_j],  // skew symmetric part of advection (standard +SUPG)
                                            _T(q    , U[_i]) += tau_ps * transpose(nabla(q)[_i]) * N(U), // Time, PSPG
                                            _T(U[_i], U[_i]) += transpose(N(U) + tau_su*U_adv*nabla(U)) * N(U), // Time, standard and SUPG
                                            _a[U[_i]] += transpose(N(U) + tau_su*U_adv*nabla(U)) * g[_i] * Tref / T + lit(m_body_force_enabler) * transpose(N(U))*g[_i]
                                    ),

                                  system_rhs += -_A * _x + _a,
                                  _A(q) = _A(q) / theta,
                                  system_matrix += invdt() * _T + theta * _A
                                )
                              )
                            ));
  //const Uint nb_regions = m_loop_regions.size();
  //for(Uint i = 0; i != nb_regions; ++i)
  //{
    //  if(i == 0)
      //    continue;

      //auto region = m_loop_regions[i];
      //auto region_action = create_proto_action(region->name(), elements_expression(/* adjoint expressions*/));
      //region_action->options().set("regions", std::vector<common::URI>({region->uri()}));
      //region_action->options().option("regions").add_tag("norecurse");
      //m_assembly->add_component(region_action);
  //}

  if(is_not_null(m_initial_conditions))
  {
    Handle<InitialConditions> solver_ic(m_initial_conditions->parent());
    cf3_assert(is_not_null(solver_ic));
    solver_ic->remove_component(*m_initial_conditions);
    m_initial_conditions = solver_ic->create_initial_condition(solution_tag());
  }

  m_update->add_component(create_proto_action("Update", nodes_expression(group
  (
    //U3 = U2,
    U2 = U1,
    U1 = U,
    U += solution(U),
    q += solution(q)
  ))));

  if(is_not_null(m_physical_model))
  {
    configure_option_recursively(solver::Tags::physical_model(), m_physical_model);
  }

  // Ensure the initial condition field list gets updated
  if(is_not_null(m_initial_conditions))
    m_initial_conditions->options().set("field_tag", solution_tag());

  if(!m_loop_regions.empty())
    configure_option_recursively(solver::Tags::regions(), std::vector<common::URI>({m_loop_regions[0]->uri()}));
}
//On region set actuator disk components
void Adjoint::on_regions_set()
{
  trigger_assembly();


}
void Adjoint::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  // Initial condition for the viscosity, defaulting to the molecular viscosity
  Handle<ProtoAction> visc_ic(initial_conditions.create_initial_condition("navier_stokes_viscosity", "cf3.solver.ProtoAction"));
  visc_ic->set_expression(nodes_expression(nu_eff = nu));

  // Initial condition for the temperature field, defaulting to the reference temperature
  Handle<ProtoAction> temp_ic(initial_conditions.create_initial_condition("boussinesq_init_tref", "cf3.solver.ProtoAction"));
  temp_ic->set_expression(nodes_expression(T = Tref));

  m_initial_conditions = initial_conditions.create_initial_condition(solution_tag());

  // Use a proto action to set the linearized_velocity easily
  Handle<ProtoAction> lin_vel_ic (initial_conditions.create_initial_condition("linearized_velocity", "cf3.solver.ProtoAction"));
  lin_vel_ic->set_expression(nodes_expression(group(U_adv = U, U1 = U, U2 = U)));
}

void Adjoint::trigger_enable_body_force()
{
  if(options().value<bool>("enable_body_force"))
  {
    m_body_force_enabler = 1.;
  }
  else
  {
    m_body_force_enabler = 0.;
  }
}
} // adjoint
} // UFEM
} // cf3
