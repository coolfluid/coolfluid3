// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "RelaxationTime.hpp"

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

common::ComponentBuilder < RelaxationTime, common::Action, LibUFEMParticles > RelaxationTime_builder;

////////////////////////////////////////////////////////////////////////////////////////////

RelaxationTime::RelaxationTime(const std::string& name) :
  ProtoAction(name)
{
  options().add("concentration_tag", "particle_concentration")
    .pretty_name("Concentration Tag")
    .description("Tag for the concentration field");
    
  options().add("concentration_variable", "c")
    .pretty_name("Concentration Variable")
    .description("Variable for the concentration variable");
    
  options().add("weighted_volume_tag", "weighted_particle_volume")
    .pretty_name("Weighted Volume Tag")
    .description("Tag for the weighted volume field");
    
  options().add("weighted_volume_variable", "zeta")
    .pretty_name("Weighted Volume Variable")
    .description("Variable for the weighted volume field");
    
  options().add("tau_variable", "zeta")
    .pretty_name("Tau Variable")
    .description("Variable for the relaxation time");

  r2.m_reference_volume = 1.;
    
  options().add("reference_volume", r2.m_reference_volume)
    .pretty_name("Reference Volume")
    .description("Reference volume, all particle volumes are divided by this")
    .link_to(&(r2.m_reference_volume));
}

RelaxationTime::~RelaxationTime()
{
}


void RelaxationTime::on_regions_set()
{
  // Fluid velocity
  FieldVariable<0, ScalarField> c(options().value<std::string>("concentration_variable"), options().value<std::string>("concentration_tag"));
  FieldVariable<1, ScalarField> zeta(options().value<std::string>("weighted_volume_variable"), options().value<std::string>("weighted_volume_tag"));
  FieldVariable<2, ScalarField> tau(options().value<std::string>("tau_variable"), "ufem_particle_relaxation_time");
  
  PhysicsConstant rho_p("particle_density");
  PhysicsConstant mu("dynamic_viscosity");

  set_expression(nodes_expression
  (
    group
    (
      tau = lit(2./9.)*rho_p/mu*lit(r2)(c, zeta)
    )
  ));
}



} // particles
} // UFEM
} // cf3
