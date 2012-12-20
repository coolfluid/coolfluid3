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

#include "ComputeFlux.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
using namespace boost::proto;
namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ComputeFlux, common::ActionDirector, LibUFEM > ComputeFlux_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

ComputeFlux::ComputeFlux(const std::string& name) :
  ActionDirector(name),
  m_rhs(options().add("lss", Handle<math::LSS::System>()).pretty_name("LSS").description("The linear system for which the boundary condition is applied")),
  h("heat_transfer_coefficient")
{
  options().add("solid_temperature_field_tag", UFEM::Tags::solution())
    .pretty_name("Solid temperature Field Tag")
    .description("Tag for the solid temperature field")
    .attach_trigger(boost::bind(&ComputeFlux::trigger_setup, this));

  options().add("fluid_temperature_field_tag", UFEM::Tags::solution())
    .pretty_name("Fluid temperature Field Tag")
    .description("Tag for the ambient fluid temperature field")
    .attach_trigger(boost::bind(&ComputeFlux::trigger_setup, this));

  // Set the gradient on the boundary elements, and configure its tag
  Handle<AdjacentCellToFace> set_boundary_gradient = create_static_component<AdjacentCellToFace>("SetBoundaryGradient");
  set_boundary_gradient->options().set("field_tag", std::string("gradient_field"));

  // The order matters here: the actions are executed in the order they are added
  // First compute ambient fluid flux...
  create_static_component<ProtoAction>("ComputeQFluid");

  // Then set it as a Neumann boundary condition
  create_static_component<ProtoAction>("NeumannHeatFlux");
}

ComputeFlux::~ComputeFlux()
{
}

void ComputeFlux::on_regions_set()
{
  get_child("NeumannHeatFlux")->options().set("regions", options()["regions"].value());
  get_child("ComputeQFluid")->options().set("regions", options()["regions"].value());
}

void ComputeFlux::trigger_setup()
{
  // Get the tags for the used fields
  const std::string solid_temperature_field_tag = options().value<std::string>("solid_temperature_field_tag");
  const std::string fluid_temperature_field_tag = options().value<std::string>("fluid_temperature_field_tag");
  //const std::string robin_flux_field_tag = options().value<std::string>("robin_flux_field_tag");

  //Handle<ProtoAction> compute_t_fluid(get_child("ComputeTFluid"));
  Handle<ProtoAction> compute_q_fluid(get_child("ComputeQFluid"));
  Handle<ProtoAction> neumann_heat_flux(get_child("NeumannHeatFlux"));

  // Represents the temperature field, as calculated
  FieldVariable<0, ScalarField> T("Temperature", solid_temperature_field_tag); // solid region

  FieldVariable<1, ScalarField> Tfl("Temperature", fluid_temperature_field_tag);

  FieldVariable<2, ScalarField> q_fluid("robin_flux", "robin_flux");

  FieldVariable<3, ScalarField> Tfluid("Temperature", "Tfluid");
//  compute_t_fluid->set_expression(nodes_expression
//   (
//     Tfl = T - (q_fluid/h) // Calculate ambient fluid temperature
//   ));

 compute_q_fluid->set_expression(nodes_expression
  (group(
    q_fluid = lit(h)*(T-Tfluid), // Calculate fluid flux applied in the Neumann condition formulation
                                    _cout << "ComputeFlux:" << "\n" << "Tfl:" << Tfl << "\n"  << "q_fluid:" <<  h*(T-Tfluid) << "\n" << "T:" << T << "\n" << "Tfluid: " << Tfluid << "\n")
  ));

 // Expression for the Neumann BC itself
 neumann_heat_flux->set_expression(elements_expression
 (
   boost::mpl::vector1<mesh::LagrangeP1::Line2D>(), // Valid for surface element types
   group(m_rhs(Tfl) += integral<1>(transpose(N(Tfl))*q_fluid*_norm(normal)), // Classical Neumann condition formulation for finite elements
   _cout << "imposed Neumann RHS: " << transpose(integral<1>(transpose(N(Tfl))*q_fluid*_norm(normal))) << "\n")
         ));


  // Raise an event to indicate that we added a variable
  common::XML::SignalOptions options;
  common::SignalArgs f = options.create_frame();
  common::Core::instance().event_handler().raise_event("ufem_variables_added", f);
}

} // namespace UFEM

} // namespace cf3
