// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "EulerDNS.hpp"

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

common::ComponentBuilder < EulerDNS, LSSActionUnsteady, LibUFEMParticles > EulerDNS_builder;

////////////////////////////////////////////////////////////////////////////////////////////

namespace detail
{
/// Evaluate the divergence of the advection operation
struct DivAdvOp
{
  // The result is a scalar
  typedef Real result_type;

  template<typename VarT>
  Real operator()(const VarT& var) const
  {
    const typename VarT::GradientT& nabla = var.nabla();
    Real result = 0.;
    for(int i = 0; i != VarT::EtypeT::dimensionality; ++i)
    {
      for(int j = 0; j != VarT::EtypeT::dimensionality; ++j)
      {
        result += nabla.row(j) * var.value().col(i) * nabla.row(i) * var.value().col(j);
      }
    }
    return result;
  }
};

static MakeSFOp<DivAdvOp>::type const div_adv = {};
}

EulerDNS::EulerDNS(const std::string& name) :
  LSSActionUnsteady(name),
  tau_p(1e-3)
{
  options().add("velocity_tag", "navier_stokes_solution")
    .pretty_name("Velocity Tag")
    .description("Tag for the field containing the velocity")
    .attach_trigger(boost::bind(&EulerDNS::trigger_set_expression, this));

  options().add("enable_body_force", false)
    .pretty_name("Enable Body Force")
    .description("Enable the body force term for the particle equation.")
    .attach_trigger(boost::bind(&EulerDNS::trigger_set_expression, this));

  options().add("theta", 0.5)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .attach_trigger(boost::bind(&EulerDNS::trigger_set_expression, this));

  options().add("tau_p", tau_p)
  .link_to(&tau_p);

  set_solution_tag("particle_concentration");

  create_component<math::LSS::ZeroLSS>("ZeroLSS");
  create_component<solver::actions::Proto::ProtoAction>("Assembly");
  create_component<BoundaryConditions>("BoundaryConditions")->set_solution_tag(solution_tag());
  create_component<math::LSS::SolveLSS>("SolveLSS");
  create_component<solver::actions::Proto::ProtoAction>("Update");

  get_child("BoundaryConditions")->mark_basic();
  
  trigger_set_expression();
}


void EulerDNS::trigger_set_expression()
{
  // Fluid velocity
  FieldVariable<0, VectorField> u("Velocity", options().value<std::string>("velocity_tag"));

  // Particle concentration
  FieldVariable<1, ScalarField> c("c", "particle_concentration");

  // Previous timestep fluid velocity
  FieldVariable<2, VectorField> u1("AdvectionVelocity1", "linearized_velocity");

  // Particle body force
  FieldVariable<3, VectorField> g("Force", "body_force");

  FieldVariable<4, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");

  // Enable o disable the body force
  const Real body_force_enable = options().value<bool>("enable_body_force") ? 1. : 0.;

  const Real theta = options().value<Real>("theta");

  // List of applicable elements
  typedef boost::mpl::vector5<
    mesh::LagrangeP1::Quad2D,
    mesh::LagrangeP1::Triag2D,
    mesh::LagrangeP1::Hexa3D,
    mesh::LagrangeP1::Tetra3D,
    mesh::LagrangeP1::Prism3D
  > AllowedElementTypesT;

  // Set the proto expression that handles the assembly
  Handle<ProtoAction>(get_child("Assembly"))->set_expression(
  elements_expression
  (
    AllowedElementTypesT(),
    group
    (
      _A = _0, _T = _0,
      compute_tau(u, nu_eff, lit(dt()), lit(tau_su)),
      element_quadrature
      (
        _A(c,c) +=  transpose(N(c) + tau_su*u*nabla(c)) * u*nabla(c) - // advection of concentration
                    lit(tau_p)*transpose(N(c)) * ((invdt()*(u - u1) + body_force_enable*g + u*nabla(u)*nodal_values(u)) * nabla(c) + detail::div_adv(u)*N(c)), // Particle inertia
        _T(c,c) +=  transpose(N(c) + tau_su*u*nabla(c)) * N(c)
      ),
      system_matrix += invdt()*_T + theta*_A,
      system_rhs += -_A*_x
    )
  ));

  // Set the proto expression for the update step
  Handle<ProtoAction>(get_child("Update"))->set_expression( nodes_expression(c += solution(c)) );
}

} // particles
} // UFEM
} // cf3
