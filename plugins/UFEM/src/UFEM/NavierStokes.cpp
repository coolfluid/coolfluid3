// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "NavierStokes.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Component.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/PropertyList.hpp"

#include "solver/actions/SolveLSS.hpp"
#include "solver/actions/ZeroLSS.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/actions/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include <solver/Tags.hpp>

#include "NavierStokesOps.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

ComponentBuilder < NavierStokes, LSSActionUnsteady, LibUFEM > NavierStokes_builder;

NavierStokes::NavierStokes(const std::string& name) :
  LSSActionUnsteady(name)
{
  options().option(solver::Tags::physical_model()).attach_trigger(boost::bind(&NavierStokes::trigger_physical_model, this));

  options().add("use_specializations", true)
    .pretty_name("Use Specializations")
    .description("Activate the use of specialized high performance code")
    .attach_trigger(boost::bind(&NavierStokes::trigger_use_specializations, this));

  set_solution_tag("navier_stokes_solution");

  trigger_use_specializations();
}

NavierStokes::~NavierStokes()
{
  if(is_not_null(m_physical_model))
  {
    physical_model().options().option("dynamic_viscosity").detach_trigger(m_viscosity_trigger_id);
    physical_model().options().option("density").detach_trigger(m_rho_trigger_id);
  }

}


void NavierStokes::trigger_physical_model()
{
  NavierStokesPhysics& phys_model = dynamic_cast<NavierStokesPhysics&>(physical_model());
  phys_model.link_properties(m_coeffs);
  m_viscosity_trigger_id = phys_model.options().option("dynamic_viscosity").attach_trigger_tracked(boost::bind(&NavierStokes::trigger_viscosity, this));
  m_rho_trigger_id = phys_model.options().option("density").attach_trigger_tracked(boost::bind(&NavierStokes::trigger_viscosity, this));
}

void NavierStokes::trigger_use_specializations()
{
  // Fist clear the structure
  std::vector<std::string> child_names;
  BOOST_FOREACH(const Component& child, *this)
  {
    child_names.push_back(child.name());
  }

  BOOST_FOREACH(const std::string& name, child_names)
  {
    remove_component(name);
  }

  const bool use_specializations = options().value<bool>("use_specializations");

  MeshTerm<1, ScalarField> p("Pressure", solution_tag());
  MeshTerm<0, VectorField> u("Velocity", solution_tag());

  MeshTerm<2, VectorField> u_adv("AdvectionVelocity", "linearized_velocity"); // The extrapolated advection velocity (n+1/2)
  MeshTerm<3, VectorField> u1("AdvectionVelocity1", "linearized_velocity");  // Two timesteps ago (n-1)
  MeshTerm<4, VectorField> u2("AdvectionVelocity2", "linearized_velocity"); // n-2
  MeshTerm<5, VectorField> u3("AdvectionVelocity3", "linearized_velocity"); // n-3
  MeshTerm<6, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");

  create_component<ZeroLSS>("ZeroLSS");
  add_component(create_proto_action("LinearizeU", nodes_expression(u_adv = 2.1875*u - 2.1875*u1 + 1.3125*u2 - 0.3125*u3)));
  if(use_specializations)
  {
    // Quads and hexas have no specialized code
    add_component(create_proto_action
    (
      "GenericAssembly",
      ns_assembly_quad_hexa_p1(*this, m_coeffs)
    ));
    // Specialized code for triags and tetras
    add_component(create_proto_action
    (
      "SpecializedAssembly",
      elements_expression
      (
        boost::mpl::vector2<mesh::LagrangeP1::Triag2D, mesh::LagrangeP1::Tetra3D>(),
        group
        (
          _A(p) = _0, _A(u) = _0, _T(p) = _0, _T(u) = _0,
          supg_specialized(p, u, u_adv, nu_eff, m_coeffs, _A, _T),
          system_matrix += invdt() * _T + 1.0 * _A,
          system_rhs += -_A * _b
        )
      )
    ));
  }
  else
  {
    add_component(create_proto_action
    (
      "GenericAssembly",
      ns_assembly_lagrange_p1(*this, m_coeffs)
    ));
  }

  Handle<BoundaryConditions> bc =  create_component<BoundaryConditions>("BoundaryConditions");
  bc->mark_basic();
  bc->set_solution_tag(solution_tag());

  create_component<SolveLSS>("SolveLSS");
  add_component(create_proto_action("Update", nodes_expression(group
  (
    u3 = u2,
    u2 = u1,
    u1 = u,
    u += solution(u),
    p += solution(p)
  ))));

  if(is_not_null(m_physical_model))
    configure_option_recursively(solver::Tags::physical_model(), m_physical_model);
}

void NavierStokes::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  m_viscosity_initial_condition = initial_conditions.create_initial_condition("navier_stokes_viscosity");
  initial_conditions.create_initial_condition(solution_tag());

  // Use a proto action to set the linearized_velocity easily
  Handle<ProtoAction> lin_vel_ic (initial_conditions.create_initial_condition("linearized_velocity", "cf3.solver.ProtoAction"));

  MeshTerm<0, VectorField> u("Velocity", solution_tag());
  MeshTerm<1, VectorField> u_adv("AdvectionVelocity", "linearized_velocity"); // The extrapolated advection velocity (n+1/2)
  MeshTerm<2, VectorField> u1("AdvectionVelocity1", "linearized_velocity");  // Two timesteps ago (n-1)
  MeshTerm<3, VectorField> u2("AdvectionVelocity2", "linearized_velocity"); // n-2
  MeshTerm<4, VectorField> u3("AdvectionVelocity3", "linearized_velocity"); // n-3

  lin_vel_ic->set_expression(nodes_expression(group(u_adv = u, u1 = u, u2 = u, u3 = u)));

  trigger_viscosity();
}

void NavierStokes::trigger_viscosity()
{
  if(is_not_null(m_viscosity_initial_condition))
  {
    m_viscosity_initial_condition->options().set("EffectiveViscosity", m_coeffs.mu / m_coeffs.rho);
  }
}



} // UFEM
} // cf3
