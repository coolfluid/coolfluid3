// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cmath>

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

#include "solver/actions/Iterate.hpp"
#include "solver/actions/NodeValence.hpp"
#include "solver/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "WALE.hpp"
#include "../Tags.hpp"

namespace cf3 {
namespace UFEM {
namespace les {

using namespace solver::actions::Proto;
using boost::proto::lit;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < WALE, common::Action, LibUFEMLES > WALE_builder;

////////////////////////////////////////////////////////////////////////////////////////////

WALE::WALE(const std::string& name) :
  ProtoAction(name),
  wale(boost::proto::as_child(m_wale_op))
{
  options().add("cw", m_wale_op.op.cw)
    .pretty_name("Cw")
    .description("Coefficient for the WALE model.")
    .link_to(&m_wale_op.op.cw)
    .mark_basic();
    
  options().add("use_anisotropic_correction", m_wale_op.op.use_anisotropic_correction)
    .pretty_name("Use anisotropic correction")
    .description("Correct the LES length scale for grid anisotropy")
    .link_to(&m_wale_op.op.use_anisotropic_correction)
    .mark_basic();

  options().add("initial_conditions", m_initial_conditions)
    .pretty_name("Initial Conditions")
    .description("The component that is used to manage the initial conditions in the solver this action belongs to")
    .link_to(&m_initial_conditions)
    .attach_trigger(boost::bind(&WALE::trigger_initial_conditions, this));
    
  options().add("velocity_tag", "navier_stokes_u_solution")
    .pretty_name("Velocity Tag")
    .description("Tag for the field containing the velocity")
    .attach_trigger(boost::bind(&WALE::trigger_set_expression, this));
    
  m_reset_viscosity = create_component<ProtoAction>("ResetViscosity");
    
  trigger_set_expression();
}

void WALE::trigger_set_expression()
{
  FieldVariable<0, VectorField> u("Velocity", options().value<std::string>("velocity_tag"));
  FieldVariable<1, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");
  FieldVariable<2, ScalarField> valence("Valence", "node_valence");
  PhysicsConstant nu_visc("kinematic_viscosity");

  // List of applicable elements
  typedef boost::mpl::vector3<
    mesh::LagrangeP1::Hexa3D,
    mesh::LagrangeP1::Tetra3D,
    mesh::LagrangeP1::Prism3D
  > AllowedElementTypesT;

  m_reset_viscosity->set_expression(nodes_expression(nu_eff = 0.));
  set_expression(elements_expression(AllowedElementTypesT(), wale(u, nu_eff, valence, nu_visc)));
}

void WALE::execute()
{
  m_reset_viscosity->execute();
  ProtoAction::execute();
}


void WALE::on_regions_set()
{
  if(is_not_null(m_node_valence))
  {
    m_node_valence->options().set("regions", options().option("regions").value());
  }
  m_reset_viscosity->options().set("regions", options().option("regions").value());
}


void WALE::trigger_initial_conditions()
{
  if(is_null(m_initial_conditions))
    return;

  if(is_null(m_node_valence))
  {
    m_node_valence = m_initial_conditions->create_initial_condition("node_valence", "cf3.solver.actions.NodeValence");
    on_regions_set();
  }
}

} // les
} // UFEM
} // cf3
