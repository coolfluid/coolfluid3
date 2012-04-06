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
  options().add_option("scalar_coefficient", 1.)
    .description("Scalar coefficient ")
    .pretty_name("Scalar coefficient")
    .link_to(&m_alpha);
    
  options().option(solver::Tags::physical_model()).attach_trigger(boost::bind(&ScalarAdvection::trigger_physical_model, this));

  // The code will only be active for these element types
  boost::mpl::vector2<mesh::LagrangeP1::Line1D,mesh::LagrangeP1::Quad2D> allowed_elements;

  m_solution_tag = "scalar_advection_solution";

  MeshTerm<0, ScalarField> Phi("Scalar", m_solution_tag);
  MeshTerm<1, VectorField> u_adv("AdvectionVelocity","linearized_velocity");

  *this
    << allocate_component<ZeroLSS>("ZeroLSS")
    << create_proto_action
    (
      "SpecializedAssembly",
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
    )
    << allocate_component<BoundaryConditions>("BoundaryConditions")
    << allocate_component<SolveLSS>("SolveLSS")
    << create_proto_action("Update", nodes_expression(Phi += solution(Phi)));

  get_child("BoundaryConditions")->handle<BoundaryConditions>()->set_solution_tag(m_solution_tag);

}

void ScalarAdvection::trigger_physical_model()
{
  dynamic_cast<NavierStokesPhysics&>(physical_model()).link_properties(m_coeffs);
}

} // UFEM
} // cf3
