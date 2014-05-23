// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "EquilibriumEulerConvergence.hpp"

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

common::ComponentBuilder < EquilibriumEulerConvergence, common::Action, LibUFEMParticles > EquilibriumEulerConvergence_builder;

////////////////////////////////////////////////////////////////////////////////////////////

struct EquilibriumEulerConvergence::ConvergenceFunctor : FunctionBase
{
  typedef void result_type;
  
  ConvergenceFunctor() :
    tau(0.01)
  {
  }
  
  template<typename VectorT>
  void operator()(const VectorT& grad_ux, const VectorT& grad_uy)
  {
    RealMatrix2 grad_u;
    grad_u.row(XX) = grad_ux;
    grad_u.row(YY) = grad_uy;
    compute_eigenvalues(grad_u);
  }

  template<typename TensorT>
  void compute_eigenvalues(const TensorT& grad_u)
  {
    ev = (0.5*(grad_u + grad_u.transpose())).template selfadjointView<Eigen::Upper>().eigenvalues();
  }

  Real tau;

  // The computed eigenvalues
  RealVector ev;
};

EquilibriumEulerConvergence::EquilibriumEulerConvergence(const std::string& name) :
  ProtoAction(name),
  m_functor(new ConvergenceFunctor())
{
  options().add("velocity_tag", "navier_stokes_solution")
    .pretty_name("Velocity Tag")
    .description("Tag for the field containing the velocity");
    
  options().add("velocity_variable", "Velocity")
    .pretty_name("Velocity Variable")
    .description("Name for the velocity variable");

  options().add("tau", m_functor->tau)
    .pretty_name("Tau")
    .description("Generalized particle relaxation time")
    .mark_basic()
    .link_to(&m_functor->tau);
    
  m_velocity_gradient = Handle<common::Action>(create_component("VelocityGradient", "cf3.UFEM.VelocityGradient"));
  m_velocity_gradient->options().set("velocity_variable", std::string("ParticleVelocity"));
  m_velocity_gradient->options().set("velocity_tag", std::string("ufem_particle_velocity"));
  m_velocity_gradient->options().set("gradient_tag", std::string("particle_velocity_gradient"));
  m_velocity_gradient->options().set("gradient_name", std::string("v"));
}

EquilibriumEulerConvergence::~EquilibriumEulerConvergence()
{
}


void EquilibriumEulerConvergence::on_regions_set()
{
  m_velocity_gradient->options().set("regions", options().option("regions").value());
  
  const Uint dim = physical_model().ndim();
  const std::string grad_tag = "particle_velocity_gradient";
  
  // Fluid velocity
  
  FieldVariable<0, VectorField> ev("ParticleVelocityGradEV", "ufem_particle_convergence");
  
  if(dim == 2)
  {
    m_functor->ev.resize(2);

    FieldVariable<1, VectorField> grad_ux("grad_vx", grad_tag);
    FieldVariable<2, VectorField> grad_uy("grad_vy", grad_tag);
    
    set_expression(nodes_expression_2d
    (
      group
      (
        lit(*m_functor)(grad_ux, grad_uy),
        ev[_i] = lit(m_functor->ev)[_i]
      )
    ));
  }
  else if(dim == 3)
  {
    m_functor->ev.resize(2);
//     FieldVariable<2, VectorField> grad_ux("grad_ux", grad_tag);
//     FieldVariable<3, VectorField> grad_uy("grad_uy", grad_tag);
//     FieldVariable<4, VectorField> grad_uz("grad_uz", grad_tag);
//     
//     set_expression(elements_expression
//     (
//       boost::mpl::vector3<mesh::LagrangeP1::Tetra3D, mesh::LagrangeP1::Hexa3D, mesh::LagrangeP1::Prism3D>(),
//       detail::set_gradient(u, valence, grad_ux, grad_uy, grad_uz)
//     ));
//     
//     m_zero_fields->set_expression(nodes_expression(group(grad_ux[_i] = 0., grad_uy[_i] = 0., grad_uz[_i] = 0.)));
  }
  else
  {
    throw common::SetupError(FromHere(), "Unsupported dimension " + common::to_str(dim) + " for EquilibriumEulerConvergence");
  }
}

void EquilibriumEulerConvergence::execute()
{
  m_velocity_gradient->execute();
  cf3::solver::actions::Proto::ProtoAction::execute();
}

} // particles
} // UFEM
} // cf3
