// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "EquilibriumEuler.hpp"

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
#include "../InitialConditions.hpp"

namespace cf3 {
namespace UFEM {
namespace particles {

using namespace solver::actions::Proto;
using boost::proto::lit;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < EquilibriumEuler, common::Action, LibUFEMParticles > EquilibriumEuler_builder;

////////////////////////////////////////////////////////////////////////////////////////////

struct EquilibriumEuler::VelocityFunctor : FunctionBase
{
  typedef void result_type;
  
  VelocityFunctor() :
    beta(0.)
  {
  }
  
  template<typename VectorT>
  void operator()(const VectorT& u, const VectorT& u1, const VectorT& g, const Real tau, const VectorT& grad_ux, const VectorT& grad_uy)
  {
    RealMatrix2 grad_u;
    grad_u.row(XX) = grad_ux;
    grad_u.row(YY) = grad_uy;
    compute_velocity(u, u1, g, tau, grad_u);
  }

  template<typename VectorT>
  void operator()(const VectorT& u, const VectorT& u1, const VectorT& g, const Real tau, const VectorT& grad_ux, const VectorT& grad_uy, const VectorT& grad_uz)
  {
    RealMatrix3 grad_u;
    grad_u.row(XX) = grad_ux;
    grad_u.row(YY) = grad_uy;
    grad_u.row(ZZ) = grad_uz;
    compute_velocity(u, u1, g, tau, grad_u);
  }

  template<typename VectorT, typename TensorT>
  void compute_velocity(const VectorT& u, const VectorT& u1, const VectorT& g, const Real tau, const TensorT& grad_u)
  {
    const TensorT invgrad = (TensorT::Identity() + tau*grad_u).inverse();
    const VectorT du_dt = (u - u1)/dt;
    v = u - tau*(1-beta)*invgrad*(du_dt + grad_u*u - g);
  }
  
  Real dt;
  
  Real beta;

  // The computed particle velocity
  RealVector v;
};

EquilibriumEuler::EquilibriumEuler(const std::string& name) :
  ProtoAction(name),
  m_functor(new VelocityFunctor())
{
  options().add("velocity_tag", "navier_stokes_solution")
    .pretty_name("Velocity Tag")
    .description("Tag for the field containing the velocity");
    
  options().add("velocity_variable", "Velocity")
    .pretty_name("Velocity Variable")
    .description("Name for the velocity variable");

  options().add("particle_velocity_variable", "ParticleVelocity")
    .pretty_name("Particle Velocity Variable")
    .description("Name for the particle velocity variable");

  options().add(solver::Tags::time(), m_time)
    .pretty_name("Time")
    .description("Component that keeps track of time for this simulation")
    .attach_trigger(boost::bind(&EquilibriumEuler::trigger_time, this))
    .link_to(&m_time);

  options().add("beta", m_functor->beta)
    .pretty_name("Beta")
    .description("Density ratio parameter")
    .mark_basic()
    .link_to(&m_functor->beta);

  options().add("tau_variable", "tau")
    .pretty_name("Tau Variable")
    .description("Field for the generalized particle relaxation time");
    
  options().add("compute_gradient", true)
    .pretty_name("Compute Gradient")
    .description("Compute the fluid velocity gradient, or rely on previously computed value");
    
  options().add("initial_conditions", Handle<UFEM::InitialConditions>())
    .pretty_name("Initial Conditions")
    .description("The component that is used to manage the initial conditions in the solver this action belongs to");
}

EquilibriumEuler::~EquilibriumEuler()
{
}


void EquilibriumEuler::on_regions_set()
{
  if(options().value<bool>("compute_gradient"))
  {
    if(is_null(m_velocity_gradient))
    {
      m_velocity_gradient = Handle<common::Action>(create_component("VelocityGradient", "cf3.UFEM.VelocityGradient"));
      m_velocity_gradient->configure_option_recursively("physical_model", options()["physical_model"].value());
      m_velocity_gradient->options().set("initial_conditions", options()["initial_conditions"].value());
      options().option("initial_conditions").link_option(m_velocity_gradient->options().option_ptr("initial_conditions"));
    }
    m_velocity_gradient->options().set("velocity_variable", options().option("velocity_variable").value());
    m_velocity_gradient->options().set("velocity_tag", options().option("velocity_tag").value());
    m_velocity_gradient->options().set("regions", options().option("regions").value());
  }
  
  const Uint dim = physical_model().ndim();
  const std::string grad_tag = "velocity_gradient";
  
  // Fluid velocity
  FieldVariable<0, VectorField> u(options().value<std::string>("velocity_variable"), options().value<std::string>("velocity_tag"));
  FieldVariable<1, VectorField> u1("AdvectionVelocity1", "linearized_velocity");
  FieldVariable<2, VectorField> v(options().value<std::string>("particle_velocity_variable"), "ufem_particle_velocity");
  FieldVariable<3, VectorField> g("Force", "body_force");
  FieldVariable<4, ScalarField> tau(options().value<std::string>("tau_variable"), "ufem_particle_relaxation_time");
  
  m_functor->v.resize(dim);

  if(dim == 2)
  {
    FieldVariable<5, VectorField> grad_ux("grad_ux", grad_tag);
    FieldVariable<6, VectorField> grad_uy("grad_uy", grad_tag);
    
    set_expression(nodes_expression_2d
    (
      group
      (
        lit(*m_functor)(u, u1, g, tau, grad_ux, grad_uy),
        v[_i] = lit(m_functor->v)[_i]
      )
    ));
  }
  else if(dim == 3)
  {
    FieldVariable<5, VectorField> grad_ux("grad_ux", grad_tag);
    FieldVariable<6, VectorField> grad_uy("grad_uy", grad_tag);
    FieldVariable<7, VectorField> grad_uz("grad_uz", grad_tag);

    set_expression(nodes_expression_3d
    (
      group
      (
        lit(*m_functor)(u, u1, g, tau, grad_ux, grad_uy, grad_uz),
        v[_i] = lit(m_functor->v)[_i]
      )
    ));
  }
  else
  {
    throw common::SetupError(FromHere(), "Unsupported dimension " + common::to_str(dim) + " for EquilibriumEuler");
  }
}

void EquilibriumEuler::execute()
{
  if(options().value<bool>("compute_gradient"))
  {
    m_velocity_gradient->execute();
  }
  else
  {
    cf3_assert(is_null(m_velocity_gradient));
  }
  cf3::solver::actions::Proto::ProtoAction::execute();
}

void EquilibriumEuler::trigger_time()
{
  if(is_null(m_time))
      return;

  m_time->options().option("time_step").attach_trigger(boost::bind(&EquilibriumEuler::trigger_timestep, this));
  trigger_timestep();
}

void EquilibriumEuler::trigger_timestep()
{
  m_functor->dt = m_time->dt();
}

} // particles
} // UFEM
} // cf3
