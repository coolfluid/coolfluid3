// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "EquilibriumEulerFEM.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Component.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/PropertyList.hpp"

#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"
#include "mesh/LagrangeP1/ElementTypes.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "../Tags.hpp"

namespace cf3 {
namespace UFEM {
namespace particles {

using namespace solver::actions::Proto;
using boost::proto::lit;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < EquilibriumEulerFEM, LSSActionUnsteady, LibUFEMParticles > EquilibriumEulerFEM_builder;

////////////////////////////////////////////////////////////////////////////////////////////

namespace detail
{
  
struct LocalImplicit
{
  typedef const RealMatrix2& result_type;
  
  template<typename UT>
  result_type operator()(RealMatrix2& result, const UT& u, const Real tau) const
  {
    result = (RealMatrix2::Identity() + tau*u.nabla()*u.value()).inverse();
    return result;
  }
};

static MakeSFOp<LocalImplicit>::type const local_implicit = {};
  
}

EquilibriumEulerFEM::EquilibriumEulerFEM(const std::string& name) :
  LSSActionUnsteady(name),
  tau(0.01),
  beta(0.)
{
  options().add("velocity_tag", "navier_stokes_solution")
    .pretty_name("Velocity Tag")
    .description("Tag for the field containing the velocity")
    .attach_trigger(boost::bind(&EquilibriumEulerFEM::trigger_set_expression, this));
    
  options().add("velocity_variable", "Velocity")
    .pretty_name("Velocity Variable")
    .description("Name for the velocity variable");
//     .attach_trigger(boost::bind(&EquilibriumEulerFEM::trigger_set_expression, this));

  options().add("beta", beta)
    .pretty_name("Beta")
    .description("Density ratio parameter")
    .mark_basic()
    .link_to(&beta);

  options().add("tau", tau)
    .pretty_name("Tau")
    .description("Generalized particle relaxation time")
    .mark_basic()
    .link_to(&tau);

  set_solution_tag("ufem_particle_velocity");

  create_component<math::LSS::ZeroLSS>("ZeroLSS");
  create_component<solver::actions::Proto::ProtoAction>("Assembly");
  create_component<BoundaryConditions>("BoundaryConditions")->set_solution_tag(solution_tag());
  create_component<math::LSS::SolveLSS>("SolveLSS");
  create_component<solver::actions::Proto::ProtoAction>("Update");

  get_child("BoundaryConditions")->mark_basic();
  
//   trigger_set_expression();
}


void EquilibriumEulerFEM::trigger_set_expression()
{
  FieldVariable<0, VectorField> u(options().value<std::string>("velocity_variable"), options().value<std::string>("velocity_tag"));
  FieldVariable<1, VectorField> u1("AdvectionVelocity1", "linearized_velocity");
  FieldVariable<2, VectorField> v("ParticleVelocity", "ufem_particle_velocity");

  // List of applicable elements
  typedef boost::mpl::vector2<
    mesh::LagrangeP1::Quad2D,
    mesh::LagrangeP1::Triag2D
  > AllowedElementTypesT;

  // Set the proto expression that handles the assembly
  Handle<ProtoAction>(get_child("Assembly"))->set_expression(
  elements_expression
  (
    AllowedElementTypesT(),
    group
    (
      _A = _0, _a = _0,
      element_quadrature
      (
        _A(v[_i],v[_i]) +=  transpose(N(v)) * N(v),
        _a[v[_i]] += transpose(transpose(N(u)) * (u - lit(tau)*(1-lit(beta))*( (u-u1)*lit(invdt()) + (u*nabla(u)*nodal_values(u) ) )*detail::local_implicit(u, lit(tau))))[_i]
      ),
      //lump(_A),
      system_matrix += _A,
      system_rhs += _a
    )
  ));

  // Set the proto expression for the update step
  Handle<ProtoAction>(get_child("Update"))->set_expression( nodes_expression(v = solution(v)) );
}

} // particles
} // UFEM
} // cf3
