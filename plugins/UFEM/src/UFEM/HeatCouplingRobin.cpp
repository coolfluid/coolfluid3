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
  system_matrix(options().option("lss")),
  h("heat_transfer_coefficient"),
  m_alpha("scalar_coefficient"),
  lambda_f("thermal_conductivity_fluid"),
  lambda_s("thermal_conductivity_solid"),
  rho("density"),
  cp("specific_heat_capacity")
{
  options().add("temperature_field_tag", UFEM::Tags::solution())
    .pretty_name("Temperature Field Tag")
    .description("Tag for the temperature field in the region where the gradient needs to be calculated")
    .attach_trigger(boost::bind(&HeatCouplingRobin::trigger_setup, this));

  options().add("temperature_fluid_field_tag", UFEM::Tags::solution())
    .pretty_name("Temperature Fluid Field Tag")
    .description("Tag for the ambient temperature field(for the Robin BC)")
    .attach_trigger(boost::bind(&HeatCouplingRobin::trigger_setup, this));

  options().add("temperature_solid_field_tag", UFEM::Tags::solution())
    .pretty_name("Temperature Solid Field Tag")
    .description("Tag for the temperature in the solid for Robin BC")
    .attach_trigger(boost::bind(&HeatCouplingRobin::trigger_setup, this));

  options().add("gradient_region", m_gradient_region)
    .pretty_name("Gradient Region")
    .description("The (volume) region in which to calculate the temperature gradient")
    .attach_trigger(boost::bind(&HeatCouplingRobin::trigger_gradient_region, this))
    .link_to(&m_gradient_region);

  // Finally set the boundary condition
  create_static_component<ProtoAction>("NeumannHeatFlux");
  create_static_component<ProtoAction>("SecondHeatFlux");

  trigger_setup();
}

HeatCouplingRobin::~HeatCouplingRobin()
{
}


void HeatCouplingRobin::on_regions_set()
{
  get_child("NeumannHeatFlux")->options().set("regions", options()["regions"].value());
  get_child("SecondHeatFlux")->options().set("regions", options()["regions"].value());
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
  const std::string temperature_solid_field_tag = options().value<std::string>("temperature_solid_field_tag");

  //Handle<AdjacentCellToFace> set_boundary_gradient(get_child("SetBoundaryGradient"));
  Handle<ProtoAction> neumann_heat_flux(get_child("NeumannHeatFlux"));
  Handle<ProtoAction> second_heat_flux(get_child("SecondHeatFlux"));

  // Represents the temperature field, as calculated
  FieldVariable<0, ScalarField> T("Temperature", temperature_field_tag);
  FieldVariable<2, ScalarField> Tsolid("Temperature", temperature_solid_field_tag);
  // Represents the gradient of the temperature, to be stored in an (element based) field
  FieldVariable<3, VectorField> GradT("TemperatureGradient", "gradient_field", mesh::LagrangeP0::LibLagrangeP0::library_namespace());


    // Expression for the Robin BC
    neumann_heat_flux->set_expression(elements_expression
    (
     boost::mpl::vector1<mesh::LagrangeP1::Line2D>(), // Valid for surface element types
     group
     (
     _A(Tsolid) = _0,
     system_matrix +=  (h * (  integral<2>(transpose(N(Tsolid))*N(Tsolid)*_norm(normal)))), // Robin system contribution
     m_rhs += (h * (integral<2>(transpose(N(T))*(T *_norm(normal))))) - (h * (  integral<2>(transpose(N(Tsolid))*Tsolid*_norm(normal)))), // First part of Tfluid calculation and Robin system contribution added to RHS (since we solve for a delta T)
     _cout << " m_rhs_first(Tfluid first part): = " << -( h * (integral<2>(transpose(N(T))*(T *_norm(normal))))) << "\n",
     _cout << " Tsolid = " << transpose(nodal_values(Tsolid)) << ", T: " << transpose(nodal_values(T)) << "\n",
     _cout << " system_matrix = " << -(h * (  integral<2>(transpose(N(Tsolid))*N(Tsolid)*_norm(normal)))) << "\n"
     )
    ));

    second_heat_flux->set_expression(elements_expression
    (
      boost::mpl::vector2<mesh::LagrangeP0::Line, mesh::LagrangeP1::Line2D>(), // Valid for surface element types
      group(m_rhs(T) += - integral<2>(transpose(N(T))*GradT*normal*lambda_f),
      _cout << "rhs_second:" << transpose(- integral<2>(transpose(N(T))*GradT*normal*lambda_f)) << "\n",
            _cout << " coordinates[0]:" << coordinates[0] << "flux:" << -GradT*normal*lambda_f/(_norm(normal)) << "\n"

            )
    ));
  // Raise an event to indicate that we added a variable (GradT)
  common::XML::SignalOptions options;
  common::SignalArgs f = options.create_frame();
  common::Core::instance().event_handler().raise_event("ufem_variables_added", f);
}

} // namespace UFEM

} // namespace cf3
