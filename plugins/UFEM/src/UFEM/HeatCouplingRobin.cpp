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

#include "HeatCouplingRobin.hpp"
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

common::ComponentBuilder < HeatCouplingRobin, common::ActionDirector, LibUFEM > HeatCouplingRobin_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

HeatCouplingRobin::HeatCouplingRobin(const std::string& name) :
  ActionDirector(name),
  m_rhs(options().add("lss", Handle<math::LSS::System>())
    .pretty_name("LSS")
    .description("The linear system for which the boundary condition is applied")),

  system_matrix(options().add("systemmatrix", Handle<math::LSS::System>())
  .pretty_name("SystemMatrix")
  .description("The linear system for which the boundary condition is applied"))
{
  options().add("gradient_region", m_gradient_region)
    .pretty_name("Gradient Region")
    .description("The (volume) region in which to calculate the temperature gradient")
    .attach_trigger(boost::bind(&HeatCouplingRobin::trigger_gradient_region, this))
    .link_to(&m_gradient_region);

  options().add("temperature_field_tag", UFEM::Tags::solution())
    .pretty_name("Temperature Field Tag")
    .description("Tag for the temperature field in the region where the gradient needs to be calculated")
    .attach_trigger(boost::bind(&HeatCouplingRobin::trigger_setup, this));

  // First compute the gradient
  create_static_component<ProtoAction>("ComputeGradient");
    // Then set the gradient on the boundary elements, and configure its tag
  Handle<AdjacentCellToFace> set_boundary_gradient = create_static_component<AdjacentCellToFace>("SetBoundaryGradient");
  set_boundary_gradient->options().set("field_tag", std::string("gradient_field"));
  // Finally set the boundary condition
  create_static_component<ProtoAction>("NeumannHeatFlux");
}

HeatCouplingRobin::~HeatCouplingRobin()

{
}


void HeatCouplingRobin::on_regions_set()
{
  Handle<AdjacentCellToFace> set_boundary_gradient(get_child("SetBoundaryGradient"));
  if(is_not_null(set_boundary_gradient))
  {
    // Set the boundary regions of the component that copies the gradient from the volume to the boundary
    set_boundary_gradient->options().set("regions", options()["regions"].value());
    // Set the regions on which to apply the Neumann BC
    get_child("NeumannHeatFlux")->options().set("regions", options()["regions"].value());
  }
}

void HeatCouplingRobin::trigger_gradient_region()
{
  Handle<Component> compute_gradient = get_child("ComputeGradient");
  if(is_not_null(compute_gradient) && is_not_null(m_gradient_region))
  {
    compute_gradient->options().set("regions", std::vector<common::URI>(1, m_gradient_region->uri()));
  }
}

void HeatCouplingRobin::trigger_setup()
{
  // Get the tags for the used fields
  const std::string temperature_field_tag = options().value<std::string>("temperature_field_tag");

  Handle<ProtoAction> compute_gradient(get_child("ComputeGradient"));
  //Handle<AdjacentCellToFace> set_boundary_gradient(get_child("SetBoundaryGradient"));
  Handle<ProtoAction> neumann_heat_flux(get_child("NeumannHeatFlux"));

  // Represents the temperature field, as calculated
  FieldVariable<0, ScalarField> T("Temperature", temperature_field_tag);
  // Represents the gradient of the temperature, to be stored in an (element based) field
  FieldVariable<1, VectorField> GradT("TemperatureGradient", "gradient_field", mesh::LagrangeP0::LibLagrangeP0::library_namespace());
  // Represents the unknown temperature field for Robin BC
  //FieldVariable<2, ScalarField> Tw("Temperature", temperature_field_tag);
  // Represents the unknown temperature field for Robin BC
  //FieldVariable<3, ScalarField> Tfl("Temperature", "Temperature in the fluid");

  // Expression to calculate the gradient, at the cell centroid:
  // nabla(T, center) is the shape function gradient matrix evaluated at the element center
  // T are the nodal values for the temperature
  compute_gradient->set_expression(elements_expression
  (
    boost::mpl::vector2<mesh::LagrangeP0::Quad, mesh::LagrangeP1::Quad2D>(),
    GradT = nabla(T, gauss_points_1)*nodal_values(T) // Calculate the gradient at the first gauss point, i.e. the cell center
  ));

 //compute_temperature_fluid->set_expression(elements_expression
 // (
 //   boost::mpl::vector2<mesh::LagrangeP0::Quad, mesh::LagrangeP1::Quad2D>(),
 //   Tfl = nodal_values(T) // get the fluid temperature
 // ));


  // Expression for the Robin BC
  neumann_heat_flux->set_expression(elements_expression
  (
    boost::mpl::vector1<mesh::LagrangeP1::Line2D>(), // Valid for surface element types
    group
    (
      system_matrix += integral<1>(transpose(N(T))*N(T)*_norm(normal)), // Formulation of Robin Boundary condition
      m_rhs += integral<1>(transpose(N(T))*T*_norm(normal))
    )
  // m_rhs(T) += integral<1>(transpose(N(T))*GradT*normal) // Formulation of Robin Boundary condition
  ));

  // Raise an event to indicate that we added a variable (GradT)
  common::XML::SignalOptions options;
  common::SignalArgs f = options.create_frame();
  common::Core::instance().event_handler().raise_event("ufem_variables_added", f);
}

} // namespace UFEM

} // namespace cf3
