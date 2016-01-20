// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "KEpsilonBase.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Component.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/PropertyList.hpp"

#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"

#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Proto/ElementGradDiv.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "LSSActionUnsteady.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {



using namespace solver::actions::Proto;
using namespace boost::proto;

KEpsilonBase::KEpsilonBase(const std::string& name) :
  solver::Action(name),
  k("k", "ke_solution"),
  epsilon("epsilon", "ke_solution"),
  nu_eff("EffectiveViscosity", "navier_stokes_viscosity"), // This is the viscosity that needs to be modified to be visible in NavierStokes
  d("wall_distance", "wall_distance"),
  yplus("yplus", "yplus")
{
  options().add("velocity_tag", "navier_stokes_solution")
    .pretty_name("Velocity Tag")
    .description("Tag for the velocity field")
    .attach_trigger(boost::bind(&KEpsilonBase::trigger_set_expression, this));

  options().add("theta", m_theta)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .link_to(&m_theta);

  options().add("minimal_viscosity_ratio", m_minimal_viscosity_ratio)
    .pretty_name("Minimal Viscosity Ratio")
    .description("Minimum valua allowed for the ratio nu_t / nu_lam used in the calculations")
    .link_to(&m_minimal_viscosity_ratio);

  options().add("l_max", m_l_max)
    .pretty_name("L Max")
    .description("Maximum value of the mixing length, used to calculate an upper bound on nu_t")
    .link_to(&m_l_max)
    .mark_basic();

  options().add("supg_type", compute_tau.data.op.supg_type_str)
    .pretty_name("SUPG Type")
    .description("Type of computation for the stabilization coefficients.")
    .link_to(&(compute_tau.data.op.supg_type_str))
    .attach_trigger(boost::bind(&ComputeTauImpl::trigger_supg_type, &compute_tau.data.op));

  options().add("u_ref", compute_tau.data.op.u_ref)
    .pretty_name("Reference velocity")
    .description("Reference velocity for the CF2 SUPG method")
    .link_to(&(compute_tau.data.op.u_ref));

  create_component<ProtoAction>("UpdateNut");

  auto lss = create_component<LSSActionUnsteady>("LSS");
  lss->set_solution_tag("ke_solution");
  lss->create_component<math::LSS::ZeroLSS>("ZeroLSS");
  lss->create_component<ProtoAction>("Assembly");
  lss->create_component<BoundaryConditions>("BoundaryConditions")->set_solution_tag("ke_solution");
  lss->get_child("BoundaryConditions")->mark_basic();
  lss->create_component<math::LSS::SolveLSS>("SolveLSS");
  lss->create_component<ProtoAction>("Update");
  lss->mark_basic();
}

KEpsilonBase::~KEpsilonBase()
{
}

void KEpsilonBase::trigger_set_expression()
{
  FieldVariable<2, VectorField> u("Velocity", options().value<std::string>("velocity_tag"));
  do_set_expressions(*Handle<LSSActionUnsteady>(get_child("LSS")), *Handle<ProtoAction>(get_child("UpdateNut")), u);
}

void KEpsilonBase::execute()
{
  auto lss_action = Handle<LSSActionUnsteady>(get_child("LSS"));
  Handle<ProtoAction> update_nut(get_child("UpdateNut"));
  update_nut->execute();
  for(Uint i = 0; i != 2; ++i)
  {
    lss_action->execute();
  }
  update_nut->execute();
}

void KEpsilonBase::on_regions_set()
{
  get_child("UpdateNut")->options().set("regions", options()["regions"].value());
  get_child("LSS")->options().set("regions", options()["regions"].value());
  access_component("LSS/BoundaryConditions")->options().set("regions", options()["regions"].value());
}

} // UFEM
} // cf3
