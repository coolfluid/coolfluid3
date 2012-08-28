// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "NavierStokesExplicitVelocity.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <boost/mpl/back_inserter.hpp>
#include <boost/mpl/copy.hpp>

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
#include "solver/Tags.hpp"

#include "NavierStokesSpecializations.hpp"
#include "SUPG.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

using boost::proto::lit;

ComponentBuilder < NavierStokesExplicitVelocity, LSSActionUnsteady, LibUFEM > NavierStokesExplicitVelocity_builder;

NavierStokesExplicitVelocity::NavierStokesExplicitVelocity(const std::string& name) :
  LSSActionUnsteady(name),
  u("Velocity", "navier_stokes_u_solution"),
  p("Pressure", "navier_stokes_p_solution"),
  u_adv("AdvectionVelocity", "linearized_velocity"),
  u1("AdvectionVelocity1", "linearized_velocity"),
  u2("AdvectionVelocity2", "linearized_velocity"),
  u3("AdvectionVelocity3", "linearized_velocity"),
  nu_eff("EffectiveViscosity", "navier_stokes_viscosity"),
  u_ref("reference_velocity"),
  rho("density"),
  nu("kinematic_viscosity")
{
  options().add("use_specializations", true)
    .pretty_name("Use Specializations")
    .description("Activate the use of specialized high performance code")
    .attach_trigger(boost::bind(&NavierStokesExplicitVelocity::trigger_assembly, this));

  set_solution_tag("navier_stokes_u_solution");

  // This ensures that the linear system matrix is reset to zero each timestep
  create_component<ZeroLSS>("ZeroLSS");
  // Extrapolate the velocity
  //add_component(create_proto_action("LinearizeU", nodes_expression(u_adv = 2.1875*u - 2.1875*u1 + 1.3125*u2 - 0.3125*u3)));

  // Container for the assembly actions. Will be filled depending on the value of options, such as using specializations or not
  m_assembly = create_component<solver::ActionDirector>("Assembly");

  // Boundary conditions
  Handle<BoundaryConditions> bc =  create_component<BoundaryConditions>("BoundaryConditions");
  bc->mark_basic();
  bc->set_solution_tag(solution_tag());

  // Solution of the LSS
  create_component<SolveLSS>("SolveLSS");

  // Update of the solution
  add_component(create_proto_action("Update", nodes_expression(group
  (
//     u3 = u2,
//     u2 = u1,
//     u1 = u,
    u += solution(u),
    _cout << "velocity update: " << transpose(solution(u)) << "\n"
  ))));

  trigger_assembly();
}

NavierStokesExplicitVelocity::~NavierStokesExplicitVelocity()
{
}


void NavierStokesExplicitVelocity::trigger_assembly()
{
  m_assembly->clear();

  // Add the assembly, depending on the use of specialized code or not
  const bool use_specializations = options().value<bool>("use_specializations");
  set_triag_assembly(use_specializations);
//   set_tetra_assembly(use_specializations);
   set_quad_assembly();
//   set_hexa_assembly();

  if(is_not_null(m_physical_model))
    configure_option_recursively(solver::Tags::physical_model(), m_physical_model);

  configure_option_recursively(solver::Tags::regions(), options().option(solver::Tags::regions()).value());
}

void NavierStokesExplicitVelocity::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  // Initial condition for the viscosity, defaulting to the molecular viscosity
  Handle<ProtoAction> visc_ic(initial_conditions.create_initial_condition("navier_stokes_viscosity", "cf3.solver.ProtoAction"));
  visc_ic->set_expression(nodes_expression(nu_eff = nu));

  initial_conditions.create_initial_condition(solution_tag());

  // Use a proto action to set the linearized_velocity easily
  Handle<ProtoAction> lin_vel_ic (initial_conditions.create_initial_condition("linearized_velocity", "cf3.solver.ProtoAction"));
  lin_vel_ic->set_expression(nodes_expression(group(u_adv = u, u1 = u, u2 = u, u3 = u)));
}

template<typename GenericElementsT, typename SpecializedElementsT>
void NavierStokesExplicitVelocity::set_assembly_expression(const std::string& action_name)
{
  // Get all the relevant types as the concatenation of the generic and specialized element types:
  typedef typename boost::mpl::copy< SpecializedElementsT, boost::mpl::back_inserter<GenericElementsT> >::type AllElementsT;

  // Proto function that applies expressions only to GenericElementsT
  static const typename boost::proto::terminal< RestrictToElementTypeTag<GenericElementsT> >::type for_generic_elements = {};
  // Proto function that applies expressions only to SpecializedElementsT
  static const typename boost::proto::terminal< RestrictToElementTypeTag<SpecializedElementsT> >::type for_specialized_elements = {};

  static boost::proto::terminal< ElementSystemMatrix< boost::mpl::int_<2> > >::type const _K = {};

  tau_su = 0.;

  // The actual matrix assembly
  m_assembly->add_component(create_proto_action
  (
    action_name,
    elements_expression
    (
      AllElementsT(),
      group
      (
        _A = _0, _T = _0, _a = _0, _K = _0,
        for_generic_elements
        (
          //compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
          element_quadrature
          (
            _K(u[_i], u[_i]) += nu_eff * transpose(nabla(u)) * nabla(u), // Diffusion
            _A(u[_i], u[_i]) += transpose(N(u) + tau_su*u_adv*nabla(u)) * u_adv*nabla(u), // Advection
            _a[u[_i]]        += transpose(N(u) + tau_su*u_adv*nabla(u)) * nabla(p)[_i] / rho * nodal_values(p), // Pressure gradient (standard and SUPG)
            //_K(u[_i], u[_j]) += transpose((tau_bulk + 0.33333333333333*nu_eff)*nabla(u)[_i]) * nabla(u)[_j], // Bulk viscosity and second viscosity effect
            //_A(u[_i], u[_j]) += transpose(0.5*u_adv[_i]*(N(u) + tau_su*u_adv*nabla(u))) * nabla(u)[_j],  // skew symmetric part of advection (standard +SUPG)
            _T(u[_i], u[_i]) += transpose(N(u) + tau_su*u_adv*nabla(u)) * N(u) // Time, standard and SUPG
          )
        ),
        system_matrix += invdt() * _T + _K,
        system_rhs += -(_A + _K) * _x - _a
      )
    )
  ));
}

void NavierStokesExplicitVelocity::set_triag_assembly(const bool use_specialization)
{
  set_assembly_expression< boost::mpl::vector1<mesh::LagrangeP1::Triag2D>, boost::mpl::vector0<> >("AssemblyTriags");
}

void NavierStokesExplicitVelocity::set_quad_assembly()
{
  set_assembly_expression< boost::mpl::vector1<mesh::LagrangeP1::Quad2D>, boost::mpl::vector0<> >("AssemblyTriags");
}

} // UFEM
} // cf3
