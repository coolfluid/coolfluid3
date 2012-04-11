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
  
  // For these elements, faster, specialized code exists
  boost::mpl::vector2<mesh::LagrangeP1::Triag2D, mesh::LagrangeP1::Tetra3D> specialized_elements;

  MeshTerm<1, ScalarField> p("Pressure", Tags::solution());
  MeshTerm<0, VectorField> u("Velocity", Tags::solution());

  MeshTerm<2, VectorField> u_adv("AdvectionVelocity", "linearized_velocity"); // The extrapolated advection velocity (n+1/2)
  MeshTerm<3, VectorField> u1("AdvectionVelocity1", "linearized_velocity");  // Two timesteps ago (n-1)
  MeshTerm<4, VectorField> u2("AdvectionVelocity2", "linearized_velocity"); // n-2
  MeshTerm<5, VectorField> u3("AdvectionVelocity3", "linearized_velocity"); // n-3

  *this
    << allocate_component<ZeroLSS>("ZeroLSS")
    << create_proto_action("LinearizeU", nodes_expression(u_adv = 2.1875*u - 2.1875*u1 + 1.3125*u2 - 0.3125*u3))
    << create_proto_action
    (
      "GenericAssembly",
      generic_ns_assembly(*this, m_coeffs)
    )
    << create_proto_action
    (
      "SpecializedAssembly",
      elements_expression
      (
        specialized_elements,
        group
        (
          _A(p) = _0, _A(u) = _0, _T(p) = _0, _T(u) = _0,
          supg_specialized(p, u, u_adv, m_coeffs, _A, _T),
          system_matrix += invdt() * _T + 1.0 * _A,
          system_rhs += -_A * _b
        )
      )
    )
    << allocate_component<BoundaryConditions>("BoundaryConditions")
    << allocate_component<SolveLSS>("SolveLSS")
    << create_proto_action("Update", nodes_expression(group
    (
      u3 = u2,
      u2 = u1,
      u1 = u,
      u += solution(u),
      p += solution(p)
    )));
}

void NavierStokes::trigger_physical_model()
{
  dynamic_cast<NavierStokesPhysics&>(physical_model()).link_properties(m_coeffs);
}


} // UFEM
} // cf3
