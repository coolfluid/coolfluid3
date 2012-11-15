// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "../NavierStokes.hpp"

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
#include "solver/actions/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "../NavierStokesSpecializations.hpp"
#include "../SUPG.hpp"
#include "../Tags.hpp"

#include "../BoussinesqAssembly.hpp"
#include "../BoussinesqAssemblyExtended.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;


ComponentBuilder < NavierStokes, LSSActionUnsteady, LibUFEM > NavierStokes_builder;

NavierStokes::NavierStokes(const std::string& name) :
  LSSActionUnsteady(name),
  u("Velocity", "navier_stokes_solution"),
  p("Pressure", "navier_stokes_solution"),
  Temp("Temperature", "navier_stokes_solution"),
  u_adv("AdvectionVelocity", "linearized_velocity"),
  u1("AdvectionVelocity1", "linearized_velocity"),
  u2("AdvectionVelocity2", "linearized_velocity"),
  u3("AdvectionVelocity3", "linearized_velocity"),
  nu_eff("EffectiveViscosity", "navier_stokes_viscosity"),
  u_ref("reference_velocity"),
  rho("density"),
  nu("kinematic_viscosity"),

  temp_ref("temp_ref"),
  rho_ref("reference_density"),
  betha("thermal_expansion_coefficient"),
  cp_heat_capacity("specific_heat_capacity"),
  kappa_heat_cond("heat_conductivity"),
  g_acceleration("gravitatonal_acceleration")

{
  options().add("use_specializations", true)
    .pretty_name("Use Specializations")
    .description("Activate the use of specialized high performance code")
    .attach_trigger(boost::bind(&NavierStokes::trigger_assembly, this));

  options().add("theta", 1.)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .attach_trigger(boost::bind(&NavierStokes::trigger_assembly, this));

  options().add("use_boussinesq", false)
    .pretty_name("Use Boussinesq")
    .description("Use the Boussinesq approximation assembly instead of the Navier-Stokes assembly")
    .attach_trigger(boost::bind(&NavierStokes::trigger_assembly, this));

  options().add("use_boussinesq_extended", false)
    .pretty_name("Use Boussinesq Extended")
    .description("Use an extended Boussinesq approximation assembly instead of the Navier-Stokes assembly")
    .attach_trigger(boost::bind(&NavierStokes::trigger_assembly, this));

  set_solution_tag("navier_stokes_solution");

  // This ensures that the linear system matrix is reset to zero each timestep
  create_component<math::LSS::ZeroLSS>("ZeroLSS");
  // Extrapolate the velocity
  add_component(create_proto_action("LinearizeU", nodes_expression(u_adv = 2.1875*u - 2.1875*u1 + 1.3125*u2 - 0.3125*u3)));

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

NavierStokes::~NavierStokes()
{
}


void NavierStokes::trigger_assembly()
{
  m_assembly->clear();
  m_update->clear();
  const bool use_boussinesq = options().value<bool>("use_boussinesq");
  if (use_boussinesq == true)
    {
    set_boussinesq_assembly_expression< boost::mpl::vector1<mesh::LagrangeP1::Quad2D>, boost::mpl::vector0<> >("BoussinesqAssemblyQuads");
    }
  else
  {
  // Add the assembly, depending on the use of specialized code or not
  const bool use_specializations = options().value<bool>("use_specializations");
  set_triag_assembly(use_specializations);
  set_tetra_assembly(use_specializations);
  set_quad_assembly();
  set_hexa_assembly();
  }
  if(is_not_null(m_physical_model))
    configure_option_recursively(solver::Tags::physical_model(), m_physical_model);

  configure_option_recursively(solver::Tags::regions(), options().option(solver::Tags::regions()).value());

  if(is_not_null(m_initial_conditions))
  {
    Handle<InitialConditions> solver_ic(m_initial_conditions->parent());
    cf3_assert(is_not_null(solver_ic));
    solver_ic->remove_component(*m_initial_conditions);
    m_initial_conditions = solver_ic->create_initial_condition(solution_tag());
  }

  if(use_boussinesq)
  {
    m_update->add_component(create_proto_action("Update", nodes_expression(group
    (
      u3 = u2,
      u2 = u1,
      u1 = u,
      u += solution(u),
      p += solution(p),
      Temp += solution(Temp)
    ))));
  }
  else
  {
    m_update->add_component(create_proto_action("Update", nodes_expression(group
    (
      u3 = u2,
      u2 = u1,
      u1 = u,
      u += solution(u),
      p += solution(p)
    ))));
  }
}

void NavierStokes::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  // Initial condition for the viscosity, defaulting to the molecular viscosity
  Handle<ProtoAction> visc_ic(initial_conditions.create_initial_condition("navier_stokes_viscosity", "cf3.solver.ProtoAction"));
  visc_ic->set_expression(nodes_expression(nu_eff = nu));

  m_initial_conditions = initial_conditions.create_initial_condition(solution_tag());

  // Use a proto action to set the linearized_velocity easily
  Handle<ProtoAction> lin_vel_ic (initial_conditions.create_initial_condition("linearized_velocity", "cf3.solver.ProtoAction"));
  lin_vel_ic->set_expression(nodes_expression(group(u_adv = u, u1 = u, u2 = u, u3 = u)));
}


} // UFEM
} // cf3
