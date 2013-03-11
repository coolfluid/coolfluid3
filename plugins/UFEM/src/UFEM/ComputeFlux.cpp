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

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;
using boost::proto::lit;

common::ComponentBuilder < ComputeFlux, common::ActionDirector, LibUFEM > ComputeFlux_Builder;

ComputeFlux::ComputeFlux(const std::string& name) :
  ActionDirector(name),
  lambda_f("thermal_conductivity_fluid"),
  lambda_s("thermal_conductivity_solid"),
  rho("density"),
  cp("specific_heat_capacity")
{

  options().add("gradient_region", m_gradient_region)
    .pretty_name("Gradient Region")
    .description("The (volume) region in which to calculate the temperature gradient")
    .attach_trigger(boost::bind(&ComputeFlux::trigger_gradient_region, this))
    .link_to(&m_gradient_region);

  options().add("temperature_field_tag", UFEM::Tags::solution())
    .pretty_name("Temperature Field Tag")
    .description("Tag for the temperature field in the region where the gradient needs to be calculated")
    .attach_trigger(boost::bind(&ComputeFlux::trigger_setup, this));

  // Compute the gradient
  create_static_component<ProtoAction>("ComputeGradient");
}

ComputeFlux::~ComputeFlux()
{
}

void ComputeFlux::trigger_gradient_region()
{
  Handle<Component> compute_gradient = get_child("ComputeGradient");
  if(is_not_null(compute_gradient) && is_not_null(m_gradient_region))
  {
    compute_gradient->options().set("regions", std::vector<common::URI>(1, m_gradient_region->uri()));
  }
}

void ComputeFlux::trigger_setup()
{
  // Get the tags for the used fields
  const std::string temperature_field_tag = options().value<std::string>("temperature_field_tag");

  Handle<ProtoAction> compute_flux_fluid(get_child("ComputeGradientFluid"));
  Handle<ProtoAction> compute_flux_solid(get_child("ComputeGradientSolid"));

  // Represents the temperature field, as calculated
  FieldVariable<0, ScalarField> T("Temperature", temperature_field_tag);
  // Represents the gradient of the temperature, to be stored in an (element based) field
  FieldVariable<1, VectorField> FluxFluid("TemperatureFluxFluid", "gradient_field", mesh::LagrangeP0::LibLagrangeP0::library_namespace());
  FieldVariable<2, VectorField> FluxSolid("TemperatureFluxSolid", "gradient_field", mesh::LagrangeP0::LibLagrangeP0::library_namespace());

  // Expression to calculate the gradient, at the cell centroid:
  // nabla(T, center) is the shape function gradient matrix evaluated at the element center
  // T are the nodal values for the temperature
  compute_flux_fluid->set_expression(elements_expression
  (
    boost::mpl::vector2<mesh::LagrangeP0::Quad, mesh::LagrangeP1::Quad2D>(),
    FluxFluid = nabla(T, gauss_points_1)*nodal_values(T)*lambda_s/(boost::proto::lit(rho)*cp) // Calculate the gradient at the first gauss point, i.e. the cell center
  ));

  compute_flux_solid->set_expression(elements_expression
  (
    boost::mpl::vector2<mesh::LagrangeP0::Quad, mesh::LagrangeP1::Quad2D>(),
    FluxSolid = nabla(T, gauss_points_1)*nodal_values(T)*lambda_f // Calculate the gradient at the first gauss point, i.e. the cell center
  ));

}

} // namespace UFEM

} // namespace cf3
