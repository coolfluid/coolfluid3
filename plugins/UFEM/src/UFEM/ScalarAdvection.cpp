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

#include "solver/actions/SolveLSS.hpp"
#include "solver/actions/ZeroLSS.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/actions/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "Tags.hpp"

#include "UFEM/NavierStokesOps.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

ComponentBuilder < ScalarAdvection, LSSActionUnsteady, LibUFEM > ScalarAdvection_builder;

ScalarAdvection::ScalarAdvection(const std::string& name) :
  LSSActionUnsteady(name)
{
  // TODO: Move this to the physical model
  options().add("scalar_coefficient", 1.)
    .description("Scalar coefficient ")
    .pretty_name("Scalar coefficient")
    .link_to(&m_alpha)
    .mark_basic();

  options().option(solver::Tags::physical_model()).attach_trigger(boost::bind(&ScalarAdvection::trigger_physical_model, this));

  options().add("scalar_name", "Scalar")
    .pretty_name("Scalar Name")
    .description("Internal (and default visible) name to use for the scalar")
    .attach_trigger(boost::bind(&ScalarAdvection::trigger_scalar_name, this));

  set_solution_tag("scalar_advection_solution");

  create_component<ZeroLSS>("ZeroLSS");
  create_component<ProtoAction>("Assembly");
  create_component<BoundaryConditions>("BoundaryConditions")->set_solution_tag(solution_tag());;
  create_component<SolveLSS>("SolveLSS");
  create_component<ProtoAction>("Update");

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

  // The code will only be active for these element types
  boost::mpl::vector2<mesh::LagrangeP1::Line1D,mesh::LagrangeP1::Quad2D> allowed_elements;

  // Scalar name is obtained from an option
  MeshTerm<0, ScalarField> Phi(options().value<std::string>("scalar_name"), solution_tag());
  MeshTerm<1, VectorField> u_adv("AdvectionVelocity","linearized_velocity");

  // Set the proto expression that handles the assembly
  Handle<ProtoAction>(get_child("Assembly"))->set_expression(
    elements_expression
    (
      // specialized_elements,
      allowed_elements,
     group
     (
       _A = _0, _T = _0,
      UFEM::compute_tau(u_adv, m_coeffs),
      element_quadrature
      (
        _A(Phi) += transpose(N(Phi)) * u_adv * nabla(Phi) + m_coeffs.tau_su * transpose(u_adv*nabla(Phi))  * u_adv * nabla(Phi) +  m_alpha * transpose(nabla(Phi)) * nabla(Phi) ,
       _T(Phi,Phi) +=  transpose(N(Phi) + m_coeffs.tau_su * u_adv * nabla(Phi)) * N(Phi)
      ),
      system_matrix += invdt() * _T + 1.0 * _A,
      system_rhs += -_A * _b
     )
    )
  );

  // Set the proto expression for the update step
  Handle<ProtoAction>(get_child("Update"))->set_expression( nodes_expression(Phi += solution(Phi)) );
}


void ScalarAdvection::trigger_physical_model()
{
  dynamic_cast<NavierStokesPhysics&>(physical_model()).link_properties(m_coeffs);
}

void ScalarAdvection::on_initial_conditions_set ( InitialConditions& initial_conditions )
{
  initial_conditions.create_initial_condition(solution_tag());
}


} // UFEM
} // cf3
