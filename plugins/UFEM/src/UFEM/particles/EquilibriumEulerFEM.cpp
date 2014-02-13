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

EquilibriumEulerFEM::EquilibriumEulerFEM(const std::string& name) :
  LSSActionUnsteady(name),
  tau_p(0.01)
{
  options().add("velocity_tag", "navier_stokes_solution")
    .pretty_name("Velocity Tag")
    .description("Tag for the field containing the velocity")
    .attach_trigger(boost::bind(&EquilibriumEulerFEM::trigger_set_expression, this));
    
  options().add("velocity_variable", "Velocity")
    .pretty_name("Velocity Variable")
    .description("Name for the velocity variable");
//     .attach_trigger(boost::bind(&EquilibriumEulerFEM::trigger_set_expression, this));

  options().add("particle_density", 1000.)
    .pretty_name("Particle Density")
    .description("Mass density for the particle")
    .mark_basic();

  options().add("particle_relaxation_time", tau_p)
    .pretty_name("Particle Relaxation Time")
    .description("Relaxation time for the particles")
    .mark_basic()
    .link_to(&tau_p);

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
    mesh::LagrangeP1::Hexa3D
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
        _a[v[_i]] += transpose(N(u)) * (u[_i] - lit(tau_p)*( (u[_i]-u1[_i])*lit(invdt()) + (u*nabla(u)*transpose(transpose(nodal_values(u))[_i]))[0] ) )
        //_a[v[_i]] += -0.5*lit(tau_p)*transpose(u[_i]*N(u)) * nabla(u)[_j] * transpose(transpose(nodal_values(u))[_j])
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
