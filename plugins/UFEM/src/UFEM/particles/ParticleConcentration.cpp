// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "ParticleConcentration.hpp"

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
#include <solver/actions/Proto/ElementGradDiv.hpp>
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

common::ComponentBuilder < ParticleConcentration, LSSActionUnsteady, LibUFEMParticles > ParticleConcentration_builder;

////////////////////////////////////////////////////////////////////////////////////////////

ParticleConcentration::ParticleConcentration(const std::string& name) :
  LSSActionUnsteady(name),
  m_theta(0.5),
  discontinuity_capture(boost::proto::as_child(m_capt_data)),
  diffusion_coeff(boost::proto::as_child(m_diff_data))
{
  options().add("velocity_tag", "ufem_particle_velocity")
    .pretty_name("Velocity Tag")
    .description("Tag for the field containing the velocity")
    .attach_trigger(boost::bind(&ParticleConcentration::trigger_set_expression, this));
    
  options().add("velocity_variable", "ParticleVelocity")
    .pretty_name("Velocity Variable")
    .description("Variable for the particle velocity")
    .attach_trigger(boost::bind(&ParticleConcentration::trigger_set_expression, this));
    
  options().add("concentration_variable", "c")
    .pretty_name("Concentration Variable")
    .description("Variable for the particle concentration")
    .attach_trigger(boost::bind(&ParticleConcentration::trigger_set_expression, this));
    
  options().add("source_term_tag", "concentration_source_terms")
    .pretty_name("Source Term Tag")
    .description("Tag for the field containing a source term")
    .attach_trigger(boost::bind(&ParticleConcentration::trigger_set_expression, this));
    
  options().add("source_term_variable", "c_src")
    .pretty_name("Source Term Variable")
    .description("Variable name for the source term")
    .attach_trigger(boost::bind(&ParticleConcentration::trigger_set_expression, this));

  options().add("theta", m_theta)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .link_to(&m_theta);

  set_solution_tag("particle_concentration");

  create_component<math::LSS::ZeroLSS>("ZeroLSS");
  create_component<solver::actions::Proto::ProtoAction>("Assembly");
  create_component<BoundaryConditions>("BoundaryConditions")->set_solution_tag(solution_tag());
  create_component<math::LSS::SolveLSS>("SolveLSS");
  create_component<solver::actions::Proto::ProtoAction>("Update");

  get_child("BoundaryConditions")->mark_basic();
      
  options().add("alpha_su", compute_tau.data.op.alpha_su)
    .pretty_name("alpha_su")
    .description("Constant to multiply the SUPG parameter with.")
    .link_to(&(compute_tau.data.op.alpha_su));
    
  options().add("supg_type", compute_tau.data.op.supg_type_str)
    .pretty_name("SUPG Type")
    .description("Type of computation for the stabilization coefficients.")
    .link_to(&(compute_tau.data.op.supg_type_str))
    .attach_trigger(boost::bind(&ComputeTauImpl::trigger_supg_type, &compute_tau.data.op));
  
  options().add("c1", compute_tau.data.op.c1)
    .pretty_name("c1")
    .description("Constant adjusting the time part of SUPG in the metric tensor formulation")
    .link_to(&(compute_tau.data.op.c1));
    
  options().add("c2", compute_tau.data.op.c2)
    .pretty_name("c2")
    .description("Constant adjusting the time part of SUPG in the metric tensor formulation")
    .link_to(&(compute_tau.data.op.c2));
    
  options().add("u_ref", compute_tau.data.op.u_ref)
    .pretty_name("Reference velocity")
    .description("Reference velocity for the CF2 SUPG method")
    .link_to(&(compute_tau.data.op.u_ref));
    
  options().add("c0", m_capt_data.op.c0)
    .pretty_name("c0")
    .description("Reference concentration for discontinuity capturing stabilization")
    .link_to(&(m_capt_data.op.c0));

  options().add("d0", m_diff_data.op.d0)
    .pretty_name("d0")
    .description("Multiplication factor for the artificial diffusion term")
    .link_to(&(m_diff_data.op.d0));
  
  trigger_set_expression();
}


void ParticleConcentration::trigger_set_expression()
{
  // Fluid velocity
  FieldVariable<0, VectorField> v(options().value<std::string>("velocity_variable"), options().value<std::string>("velocity_tag"));

  // Particle concentration
  FieldVariable<1, ScalarField> c(options().value<std::string>("concentration_variable"), solution_tag());
  
  // Source term
  FieldVariable<2, ScalarField> s(options().value<std::string>("source_term_variable"), options().value<std::string>("source_term_tag"));

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
      _A = _0, _T = _0, _a = _0,
      compute_tau.apply(v, 0., lit(dt()), lit(tau_su)),
      //discontinuity_capture(v, c, lit(tau_dc)),
      element_quadrature
      (
        _A(c,c) +=  transpose(N(c) + (lit(tau_su)*v + discontinuity_capture(v, c)*transpose(gradient(c)))*nabla(c)) * (v*nabla(c) + divergence(v)*N(c)) + diffusion_coeff(v,c)*transpose(nabla(c))*nabla(c),
        _T(c,c) +=  transpose(N(c) + (lit(tau_su)*v + discontinuity_capture(v, c)*transpose(gradient(c)))*nabla(c)) * N(c),
        _a[c]   +=  transpose(N(c) + (lit(tau_su)*v + discontinuity_capture(v, c)*transpose(gradient(c)))*nabla(c)) * s
      ),
      system_matrix += invdt()*_T + theta*_A,
      system_rhs += -_A*_x + _a
    )
  ));

  // Set the proto expression for the update step
  Handle<ProtoAction>(get_child("Update"))->set_expression( nodes_expression(c = _max(c + solution(c), 0.)) );
}

} // particles
} // UFEM
} // cf3
