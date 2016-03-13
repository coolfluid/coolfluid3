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
  U("AdjVelocity", "adjoint_solution"),
  q("AdjPressure", "adjoint_solution"),
  U_adv("AdjAdvectionVelocity", "adj_linearized_velocity"),
  U1("AdjAdvectionVelocity1", "adj_linearized_velocity"),
  U2("AdjAdvectionVelocity2", "adj_linearized_velocity"),
  U3("AdjAdvectionVelocity3", "adj_linearized_velocity"),
  nu_eff("EffectiveViscosity", "navier_stokes_viscosity"),
  density_ratio("density_ratio", "density_ratio"),
  g("Force", "body_force"),
  rho("density"),
  nu("kinematic_viscosity")
{
  const std::vector<std::string> restart_field_tags = boost::assign::list_of("navier_stokes_solution")("adjoint_solution")("adj_linearized_velocity")("navier_stokes_viscosity");
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

  options().add("ct", ct)
    .pretty_name("Reference velocity")
    .description("Reference velocity for the CF2 SUPG method")
    .link_to(&ct);

  options().add("theta", theta)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .link_to(&theta);

  options().option("regions").add_tag("norecurse");

  set_solution_tag("adjoint_solution");

  // This ensures that the linear system matrix is reset to zero each timestep
  create_component<math::LSS::ZeroLSS>("ZeroLSS")->options().set("reset_solution", false);
  // Extrapolate the velocity
  add_component(create_proto_action("LinearizeU", nodes_expression(U_adv = 2.1875*U - 2.1875*U1 + 1.3125*U2 - 0.3125*U3)));
  //add_component(create_proto_action("LinearizeU", nodes_expression(U_adv = U)));

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
                                boost::mpl::vector2<
                                    mesh::LagrangeP1::Triag2D,
                                    mesh::LagrangeP1::Tetra3D
                                    >(),
                                group
                                (
                                  _A = _0, _T = _0, _a = _0,
                                    compute_tau.apply(U_adv, nu_eff, lit(dt()), lit(tau_ps), lit(tau_su), lit(tau_bulk)),
                                    element_quadrature
                                    (
                                            _A(q    , U[_i]) += transpose(N(q) + tau_ps*U_adv*nabla(q)*0.5) * nabla(U)[_i] + tau_ps * transpose(nabla(q)[_i]) * U_adv*nabla(U), // Standard continuity + PSPG for advection
                                            _A(q    , q)     += tau_ps * transpose(nabla(q)) * nabla(q), // Continuity, PSPG
                                            _A(U[_i], U[_i]) += nu_eff * transpose(nabla(U)) * nabla(U) + transpose(N(U) + tau_su*U_adv*nabla(U)) * U_adv*nabla(U) - transpose(transpose(N(u) + tau_su*U_adv*nabla(U)) * U_adv*nabla(U)), //Diffusion + advection
                                            _A(U[_i], q)     += transpose(N(U) + tau_su*U_adv*nabla(U)) * nabla(q)[_i], // Pressure gradient (standard and SUPG)
                                            _A(U[_i], U[_j]) += transpose(tau_bulk*nabla(U)[_i] // Bulk viscosity
                                                                + 0.5*U_adv[_i]*(N(U) + tau_su*U_adv*nabla(U))) * nabla(U)[_j],  // skew symmetric part of advection (standard +SUPG)
                                            _T(q    , U[_i]) += tau_ps * transpose(nabla(q)[_i]) * N(U), // Time, PSPG
                                            _T(U[_i], U[_i]) += transpose(N(U) + tau_su*U_adv*nabla(U)) * N(U) // Time, standard and SUPG
                                    ),
                                  system_rhs += -_A * _x,
                                  _A(q) = _A(q) / theta,
                                  system_matrix += invdt() * _T + theta * _A
                                )
                              )
                            ));
  for(auto&& region : m_actuator_regions)
  {
      auto region_action = create_proto_action(region->name(), elements_expression(boost::mpl::vector2<mesh::LagrangeP1::Triag2D, mesh::LagrangeP1::Tetra3D>(), group(
                                                      _a[U] = _0, // set element vector to zero
                                                      element_quadrature(_a[U[0]] +=  transpose(N(U) + tau_su*U_adv*nabla(U)) * u[0] * u[0]* density_ratio * 6 * 0.071 * (1-0.071)/0.5,
                                                                         _A(U[0], U[0]) += transpose(N(U))*N(u)*4 * 0.071/(1-0.071)/0.5
                                                                                       ), // integrate
                                                      system_rhs +=-_A * _x + _a // update global system RHS with element vector
                                                   )));
      m_assembly->add_component(region_action);
      region_action->options().set("regions", std::vector<common::URI>({region->uri()}));
      region_action->options().option("regions").add_tag("norecurse");
  }

  m_update->add_component(create_proto_action("Update", nodes_expression(group
  (
    U3 = U2,
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
}

//On region set actuator disk components
void Adjoint::on_regions_set()
{
  if(m_loop_regions.empty())
  {
    return; // no regions -> do nothing
  }

  // Put all regions except the first one in m_actuator_regions
  m_actuator_regions.clear();
  m_actuator_regions.insert(m_actuator_regions.end(), m_loop_regions.begin()+1, m_loop_regions.end());

  // Remove all except the first region from m_loop_regions
  m_loop_regions.resize(1);

  trigger_assembly();
  LSSActionUnsteady::on_regions_set();
}

void Adjoint::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  m_initial_conditions = initial_conditions.create_initial_condition(solution_tag());

  // Use a proto action to set the linearized_velocity easily
  Handle<ProtoAction> lin_vel_ic (initial_conditions.create_initial_condition("adj_linearized_velocity", "cf3.solver.ProtoAction"));
  lin_vel_ic->set_expression(nodes_expression(group(U_adv = U, U1 = U, U2 = U, U3 = U)));
}
} // adjoint
} // UFEM
} // cf3
