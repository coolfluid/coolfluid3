// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "ScalarAdvection.hpp"

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

ComponentBuilder < ScalarAdvection, LSSActionUnsteady, LibUFEM > ScalarAdvection_builder;

////////////////////////////////////////////////////////////////////////////////////////////

ScalarAdvection::ScalarAdvection(const std::string& name) :
  LSSActionUnsteady(name),
  m_theta(0.5),
  m_pr(1.),
  m_pr_t(1.)
{
  options().add("pr", m_pr)
    .description("Pr")
    .pretty_name("Prandtl number")
    .link_to(&m_pr)
    .mark_basic();

  options().add("pr_t", m_pr_t)
    .description("Pr_t")
    .pretty_name("Turbulent Prandtl number")
    .link_to(&m_pr_t)
    .mark_basic();

  options().add("theta", m_theta)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .link_to(&m_theta);

  options().add("scalar_name", "Scalar")
    .pretty_name("Scalar Name")
    .description("Internal (and default visible) name to use for the scalar")
    .attach_trigger(boost::bind(&ScalarAdvection::trigger_scalar_name, this));

  options().add("velocity_tag", "navier_stokes_solution")
    .pretty_name("Velocity Tag")
    .description("Tag for the velocity field")
    .attach_trigger(boost::bind(&ScalarAdvection::trigger_scalar_name, this));

  set_solution_tag("scalar_advection_solution");

  create_component<math::LSS::ZeroLSS>("ZeroLSS");
  create_component<ProtoAction>("Assembly");
  create_component<BoundaryConditions>("BoundaryConditions")->set_solution_tag(solution_tag());
  create_component<math::LSS::SolveLSS>("SolveLSS");
  create_component<ProtoAction>("Update");

  get_child("BoundaryConditions")->mark_basic();

  // Set the default scalar name
  trigger_scalar_name();
}

void ScalarAdvection::trigger_scalar_name()
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
  FieldVariable<0, ScalarField> phi(options().value<std::string>("scalar_name"), solution_tag());
  FieldVariable<1, VectorField> u("Velocity",options().value<std::string>("velocity_tag"));
  FieldVariable<2, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");
  FieldVariable<3, ScalarField> temperature1_sa("TemperatureHistorySA1", "temperature_history_sa");
  FieldVariable<4, ScalarField> temperature2_sa("TemperatureHistorySA2", "temperature_history_sa");
  FieldVariable<5, ScalarField> temperature3_sa("TemperatureHistorySA3", "temperature_history_sa");

  PhysicsConstant nu_lam("kinematic_viscosity");

  ConfigurableConstant<Real> relaxation_factor_scalar("relaxation_factor_scalar", "factor for relaxation in case of coupling", 1.);

  // Set the proto expression that handles the assembly
  Handle<ProtoAction>(get_child("Assembly"))->set_expression(
    elements_expression
    (
      AllowedElementTypesT(),
      group
      (
        _A = _0, _T = _0,
        compute_tau.apply(u, nu_eff, lit(dt()), lit(tau_su)),
        element_quadrature
        (
          _A(phi) += transpose(N(phi) + (tau_su*u + cw.apply(u, phi))*nabla(phi)) * u * nabla(phi) +  (nu_lam / lit(m_pr) + (nu_eff - nu_lam) / lit(m_pr_t)) * transpose(nabla(phi)) * nabla(phi),
          _T(phi,phi) +=  transpose(N(phi) + (tau_su*u + cw.apply(u, phi))*nabla(phi)) * N(phi)
        ),
        system_matrix += invdt() * _T + m_theta * _A,
        system_rhs += -_A * _x
      )
    )
  );

  // Set the proto expression for the update step
  Handle<ProtoAction>(get_child("Update"))->set_expression( nodes_expression(phi += relaxation_factor_scalar * solution(phi)) );
}

} // UFEM
} // cf3
