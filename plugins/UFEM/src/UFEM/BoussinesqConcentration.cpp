// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "BoussinesqConcentration.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

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

#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

using boost::proto::lit;

////////////////////////////////////////////////////////////////////////////////////////////

ComponentBuilder < BoussinesqConcentration, LSSActionUnsteady, LibUFEM > BoussinesqConcentration_builder;

////////////////////////////////////////////////////////////////////////////////////////////

BoussinesqConcentration::BoussinesqConcentration(const std::string& name) :
  LSSActionUnsteady(name)
{
  options().add("theta", m_theta)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .link_to(&m_theta);

  options().add("velocity_tag", "navier_stokes_solution")
    .pretty_name("Velocity Tag")
    .description("Tag for the velocity field")
    .attach_trigger(boost::bind(&BoussinesqConcentration::trigger_assembly, this));

  options().add("density", density)
    .pretty_name("Density")
    .description("Density of the dispersed compound.")
    .link_to(&density)
    .mark_basic();

  set_solution_tag("volume_fraction_boussinesq");

  create_component<math::LSS::ZeroLSS>("ZeroLSS");
  create_component<ProtoAction>("Assembly");
  create_component<BoundaryConditions>("BoundaryConditions")->set_solution_tag(solution_tag());
  create_component<math::LSS::SolveLSS>("SolveLSS");
  create_component<ProtoAction>("Update");

  get_child("BoundaryConditions")->mark_basic();

  // Do the assembly
  trigger_assembly();
}

void BoussinesqConcentration::trigger_assembly()
{
  // Make sure variables aren't registered multiple times
  if(is_not_null(m_physical_model))
  {
    if(is_not_null(m_physical_model->variable_manager().get_child(solution_tag())))
      m_physical_model->variable_manager().remove_component(solution_tag());
  }

  // List of applicable elements
  typedef boost::mpl::vector5<
    mesh::LagrangeP1::Quad2D,
    mesh::LagrangeP1::Triag2D,
    mesh::LagrangeP1::Hexa3D,
    mesh::LagrangeP1::Tetra3D,
    mesh::LagrangeP1::Prism3D
  > AllowedElementTypesT;

  // Scalar name is obtained from an option
  FieldVariable<0, ScalarField> c(solution_tag(), solution_tag());
  FieldVariable<1, VectorField> u("Velocity",options().value<std::string>("velocity_tag"));
  FieldVariable<2, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");
  FieldVariable<3, ScalarField> density_ratio("density_ratio", "density_ratio");

  PhysicsConstant nu_lam("kinematic_viscosity");
  PhysicsConstant D("D");
  PhysicsConstant Sc_t("Sc_t");
  PhysicsConstant rho("density"); // Density of the carrier fluid

  // Set the proto expression that handles the assembly
  Handle<ProtoAction>(get_child("Assembly"))->set_expression(
  elements_expression
  (
    AllowedElementTypesT(),
    group
    (
      _A(c) = _0, _T(c) = _0,
      compute_tau.apply(u, nu_eff, lit(dt()), lit(tau_su)),
      element_quadrature
      (
        _A(c) += transpose(N(c) + (tau_su*u + cw.apply(u, c))*nabla(c)) * u * nabla(c) +  (lit(D) + (nu_eff - nu_lam) / lit(Sc_t)) * transpose(nabla(c)) * nabla(c),
        _T(c,c) +=  transpose(N(c) + (tau_su*u + cw.apply(u, c))*nabla(c)) * N(c)
      ),
      system_matrix += invdt() * _T + m_theta * _A,
      system_rhs += -_A * _x
    )
  ));

  // Set the proto expression for the update step
  Handle<ProtoAction>(get_child("Update"))->set_expression(nodes_expression(group
  (
    c += solution(c),
    density_ratio = lit(1.) - rho / (c*density + (1.-c)*rho)
  )));
}

void BoussinesqConcentration::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  initial_conditions.create_initial_condition(solution_tag());
}

} // UFEM
} // cf3
