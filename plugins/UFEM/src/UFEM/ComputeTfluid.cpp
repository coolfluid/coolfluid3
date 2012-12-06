// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include <common/EventHandler.hpp>

#include "math/LSS/System.hpp"

#include "mesh/Region.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"

#include "ComputeTfluid.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ComputeTfluid, common::ActionDirector, LibUFEM > ComputeTfluid_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

ComputeTfluid::ComputeTfluid(const std::string& name) :

  h("heat_transfer_coefficient"),
  ActionDirector(name),
  m_rhs(options().add("lss", Handle<math::LSS::System>()).pretty_name("LSS").description("The linear system for which the boundary condition is applied"))
{

  options().add("temperature_field_tag", UFEM::Tags::solution())
    .pretty_name("Temperature Field Tag")
    .description("Tag for the temperature field in the region where the gradient needs to be calculated")
    .attach_trigger(boost::bind(&ComputeTfluid::trigger_setup, this));

  options().add("temperature_fluid_field_tag", UFEM::Tags::solution())
    .pretty_name("Temperature Fluid Field Tag")
    .description("Tag for the ambient fluid temperature field")
    .attach_trigger(boost::bind(&ComputeTfluid::trigger_setup, this));

//  options().add("robin_flux_field_tag", 0.)
//    .pretty_name("Robin Flux")
//    .description("Tag for heat flux evaluated by heat transfer equation")
//    .attach_trigger(boost::bind(&ComputeTfluid::trigger_setup, this));

  // Compute ambient fluid temperature
  create_static_component<ProtoAction>("ComputeTFluid");
}

ComputeTfluid::~ComputeTfluid()
{
}

void ComputeTfluid::trigger_setup()
{
  // Get the tags for the used fields
  const std::string temperature_field_tag = options().value<std::string>("temperature_field_tag");
  const std::string temperature_fluid_field_tag = options().value<std::string>("temperature_fluid_field_tag");
 // const std::string robin_flux_field_tag = options().value<std::string>("robin_flux_field_tag");

  Handle<ProtoAction> compute_t_fluid(get_child("ComputeTFluid"));

  // Represents the temperature field, as calculated
  FieldVariable<0, ScalarField> T("Temperature", temperature_field_tag); // solid region

  FieldVariable<1, ScalarField> Tfl("Temperature", temperature_fluid_field_tag);

  FieldVariable<2, ScalarField> q_fluid("robin_flux", "robin_flux");

 compute_t_fluid->set_expression(nodes_expression
                                 (group(
    Tfl = T - (q_fluid/h), // Calculate fluid flux applied in the Neumann condition formulation
                                    _cout << "Tfl:" << T - (q_fluid/h) << "\n")
  ));

  // Raise an event to indicate that we added a variable
  common::XML::SignalOptions options;
  common::SignalArgs f = options.create_frame();
  common::Core::instance().event_handler().raise_event("ufem_variables_added", f);
}

} // namespace UFEM

} // namespace cf3
