// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "../NavierStokesSemiImplicit.hpp"

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
#include "math/LSS/Trilinos/TrilinosCrsMatrix.hpp"
#include "math/LSS/Trilinos/TekoBlockedOperator.hpp"
#include "math/LSS/Trilinos/TrilinosVector.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "../../Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

using boost::proto::lit;

ComponentBuilder < NavierStokesSemiImplicit, common::ActionDirector, LibUFEM > NavierStokesSemiImplicit_builder;

NavierStokesSemiImplicit::NavierStokesSemiImplicit(const std::string& name) :
  LSSActionUnsteady(name),
  u("Velocity", "navier_stokes_solution"),
  p("Pressure", "navier_stokes_solution"),
  u_adv("AdvectionVelocity", "linearized_velocity"),
  u1("AdvectionVelocity1", "linearized_velocity"),
  u2("AdvectionVelocity2", "linearized_velocity"),
  u3("AdvectionVelocity3", "linearized_velocity"),
  nu_eff("EffectiveViscosity", "navier_stokes_viscosity"),
  u_ref("reference_velocity"),
  nu("kinematic_viscosity")
{
  options().add("theta", 1.)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .link_to(&theta);
  
  set_solution_tag("navier_stokes_solution");
  options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));
  options().set("solution_strategy", std::string("cf3.UFEM.SegregatedSolveStrategy"));
  mark_basic();
  
  add_component(create_proto_action("LinearizeU", nodes_expression(u_adv = 2.1875*u - 2.1875*u1 + 1.3125*u2 - 0.3125*u3)));
  create_component<math::LSS::ZeroLSS>("ZeroLSS");
  
  m_rhs_lss = create_component<LSSAction>("RhsLSS");
  Handle<math::LSS::ZeroLSS> zero_rhs_lss = m_rhs_lss->create_component<math::LSS::ZeroLSS>("ZeroLSS");
  zero_rhs_lss->options().option("lss").add_tag("norecurse");
  m_rhs_lss->options().option("lss").add_tag("norecurse");
  m_rhs_lss->set_solution_tag(solution_tag());
  Handle<math::LSS::System> rhs_lss = m_rhs_lss->create_component<math::LSS::System>("LSS");
  rhs_lss->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));
  m_rhs_lss->options().set("lss", rhs_lss);
  zero_rhs_lss->options().set("lss", rhs_lss);

  m_t_lss = create_component<LSSAction>("TLSS");
  Handle<math::LSS::ZeroLSS> zero_t_lss = m_t_lss->create_component<math::LSS::ZeroLSS>("ZeroLSS");
  zero_t_lss->options().option("lss").add_tag("norecurse");
  m_t_lss->options().option("lss").add_tag("norecurse");
  m_t_lss->set_solution_tag(solution_tag());
  Handle<math::LSS::System> t_lss = m_t_lss->create_component<math::LSS::System>("LSS");
  t_lss->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));
  m_t_lss->options().set("lss", t_lss);
  zero_t_lss->options().set("lss", t_lss);
  
  set_matrix_assembly_quad(*m_rhs_lss, *m_t_lss);
  set_matrix_assembly_triag(*m_rhs_lss, *m_t_lss);
  
  // Apply BC
  Handle<BoundaryConditions> bc =  create_component<BoundaryConditions>("BC");
  bc->mark_basic();
  bc->set_solution_tag(solution_tag());

  create_component<ProtoAction>("SetSolution")->set_expression(nodes_expression(group(solution(u) = u, solution(p) = p)));
  
  create_component<math::LSS::SolveLSS>("SolveLSS");
  create_component<ProtoAction>("Update")->set_expression(nodes_expression(group(u3 = u2, u2 = u1, u1 = u, u = solution(u), p = solution(p))));

  
  if(is_not_null(m_physical_model))
    configure_option_recursively(solver::Tags::physical_model(), m_physical_model);

  configure_option_recursively(solver::Tags::regions(), options().option(solver::Tags::regions()).value());
}

NavierStokesSemiImplicit::~NavierStokesSemiImplicit()
{
}

void NavierStokesSemiImplicit::execute()
{
  if(is_null(m_variables_descriptor))
  {
    access_component("LSS/SolutionStrategy")->options().set("theta", theta);
    access_component("LSS/SolutionStrategy")->options().set("rhs_system", Handle<math::LSS::System>(m_rhs_lss->get_child("LSS")));
    access_component("LSS/SolutionStrategy")->options().set("t_system", Handle<math::LSS::System>(m_t_lss->get_child("LSS")));
    m_variables_descriptor = common::find_component_ptr_with_tag<math::VariablesDescriptor>(physical_model().variable_manager(), "navier_stokes_solution");
    access_component("LSS/SolutionStrategy")->options().set("variables_descriptor", m_variables_descriptor);
  }
  solver::ActionDirector::execute();
}

void NavierStokesSemiImplicit::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  // Initial condition for the viscosity, defaulting to the molecular viscosity
  Handle<ProtoAction> visc_ic(initial_conditions.create_initial_condition("navier_stokes_viscosity", "cf3.solver.ProtoAction"));
  visc_ic->set_expression(nodes_expression(nu_eff = nu));

  // Use a proto action to set the linearized_velocity easily
  Handle<ProtoAction> lin_vel_ic (initial_conditions.create_initial_condition("linearized_velocity", "cf3.solver.ProtoAction"));
  lin_vel_ic->set_expression(nodes_expression(group(u_adv = u, u1 = u, u2 = u, u3 = u)));
}

void NavierStokesSemiImplicit::on_regions_set()
{
  LSSActionUnsteady::on_regions_set();
  m_rhs_lss->options().set("lss", Handle<math::LSS::System>(m_rhs_lss->get_child("LSS")));
  m_t_lss->options().set("lss", Handle<math::LSS::System>(m_t_lss->get_child("LSS")));
}

} // UFEM
} // cf3
