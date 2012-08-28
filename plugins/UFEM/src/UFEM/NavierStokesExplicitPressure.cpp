// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "NavierStokesExplicitPressure.hpp"

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

ComponentBuilder < NavierStokesExplicitPressure, LSSActionUnsteady, LibUFEM > NavierStokesExplicitPressure_builder;

NavierStokesExplicitPressure::NavierStokesExplicitPressure(const std::string& name) :
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
    .attach_trigger(boost::bind(&NavierStokesExplicitPressure::trigger_assembly, this));

  set_solution_tag("navier_stokes_p_solution");

  // This ensures that the linear system matrix is reset to zero each timestep
  create_component<ZeroLSS>("ZeroLSS");
  // Extrapolate the velocity
//   add_component(create_proto_action("LinearizeU", nodes_expression(u_adv = 2.1875*u - 2.1875*u1 + 1.3125*u2 - 0.3125*u3)));

  // Container for the assembly actions. Will be filled depending on the value of options, such as using specializations or not
  m_assembly = create_component<solver::ActionDirector>("Assembly");

  // Boundary conditions
  Handle<BoundaryConditions> bc =  create_component<BoundaryConditions>("BoundaryConditions");
  bc->mark_basic();
  bc->set_solution_tag(solution_tag());

  // Solution of the LSS
  create_component<SolveLSS>("SolveLSS");

  // Update of the solution
  add_component(create_proto_action("Update", nodes_expression(p += solution(p))));

  trigger_assembly();
}

NavierStokesExplicitPressure::~NavierStokesExplicitPressure()
{
}


void NavierStokesExplicitPressure::trigger_assembly()
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

void NavierStokesExplicitPressure::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  initial_conditions.create_initial_condition(solution_tag());
}

template<typename GenericElementsT, typename SpecializedElementsT>
void NavierStokesExplicitPressure::set_assembly_expression(const std::string& action_name)
{
  // Get all the relevant types as the concatenation of the generic and specialized element types:
  typedef typename boost::mpl::copy< SpecializedElementsT, boost::mpl::back_inserter<GenericElementsT> >::type AllElementsT;

  // Proto function that applies expressions only to GenericElementsT
  static const typename boost::proto::terminal< RestrictToElementTypeTag<GenericElementsT> >::type for_generic_elements = {};
  // Proto function that applies expressions only to SpecializedElementsT
  static const typename boost::proto::terminal< RestrictToElementTypeTag<SpecializedElementsT> >::type for_specialized_elements = {};

  // The actual matrix assembly
  m_assembly->add_component(create_proto_action
  (
    action_name,
    elements_expression
    (
      AllElementsT(),
      group
      (
        _A = _0, _a = _0,
        for_generic_elements
        (
          compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
          element_quadrature
          (
            _A(p    , p)  += tau_ps * transpose(nabla(p)) * nabla(p) / rho, // Continuity, PSPG
            _a[p]         += (tau_ps * transpose(nabla(p)[_i]) * N(u) * invdt()
	                  +  transpose(N(p)) * nabla(u)[_i] + tau_ps * transpose(nabla(p)[_i]) * u_adv*nabla(u)) * transpose(transpose(nodal_values(u))[_i])
          )
        ),
        system_matrix += _A,
        system_rhs += -_a - _A*_x
      )
    )
  ));
}

void NavierStokesExplicitPressure::set_triag_assembly(const bool use_specialization)
{
  set_assembly_expression< boost::mpl::vector1<mesh::LagrangeP1::Triag2D>, boost::mpl::vector0<> >("AssemblyTriags");
}

void NavierStokesExplicitPressure::set_quad_assembly()
{
  set_assembly_expression< boost::mpl::vector1<mesh::LagrangeP1::Quad2D>, boost::mpl::vector0<> >("AssemblyQuads");
}

} // UFEM
} // cf3
