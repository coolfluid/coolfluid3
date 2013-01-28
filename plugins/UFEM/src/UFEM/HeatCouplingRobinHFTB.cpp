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

#include "HeatCouplingRobinHFTB.hpp"
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

common::ComponentBuilder < HeatCouplingRobinHFTB, common::ActionDirector, LibUFEM > HeatCouplingRobinHFTB_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

HeatCouplingRobinHFTB::HeatCouplingRobinHFTB(const std::string& name) :
  ActionDirector(name),
  m_rhs(options().add("lss", Handle<math::LSS::System>())
    .pretty_name("LSS")
    .description("The linear system for which the boundary condition is applied")),
  system_matrix(options().option("lss")),
  h("heat_transfer_coefficient"),
  m_alpha("scalar_coefficient")
{
  options().add("temperature_field_tag", UFEM::Tags::solution())
    .pretty_name("Temperature Field Tag")
    .description("Tag for the temperature field in the region where the gradient needs to be calculated")
    .attach_trigger(boost::bind(&HeatCouplingRobinHFTB::trigger_setup, this));

  options().add("temperature_fluid_field_tag", UFEM::Tags::solution())
    .pretty_name("Temperature Fluid Field Tag")
    .description("Tag for the ambient temperature field(for the Robin BC)")
    .attach_trigger(boost::bind(&HeatCouplingRobinHFTB::trigger_setup, this));

  options().add("temperature_solid_field_tag", UFEM::Tags::solution())
    .pretty_name("Temperature Solid Field Tag")
    .description("Tag for the temperature in the solid for Robin BC")
    .attach_trigger(boost::bind(&HeatCouplingRobinHFTB::trigger_setup, this));

  options().add("robin_pre", true)
    .pretty_name("Robin precalculation")
    .description("Do a first of two steps to calculate an 'ambient' fluid temperature for the Robin BC")
    .attach_trigger(boost::bind(&HeatCouplingRobinHFTB::trigger_setup, this));

  options().add("gradient_region", m_gradient_region)
    .pretty_name("Gradient Region")
    .description("The (volume) region in which to calculate the temperature gradient")
    .attach_trigger(boost::bind(&HeatCouplingRobinHFTB::trigger_gradient_region, this))
    .link_to(&m_gradient_region);

  // Finally set the boundary condition
  create_static_component<ProtoAction>("NeumannHeatFlux");
  create_static_component<ProtoAction>("SecondHeatFlux");

  trigger_setup();
}

HeatCouplingRobinHFTB::~HeatCouplingRobinHFTB()
{
}


void HeatCouplingRobinHFTB::on_regions_set()
{
  get_child("NeumannHeatFlux")->options().set("regions", options()["regions"].value());
  get_child("SecondHeatFlux")->options().set("regions", options()["regions"].value());
}

void HeatCouplingRobinHFTB::trigger_gradient_region()
{
  Handle<Component> compute_gradient = get_child("ComputeGradient");
  if(is_not_null(compute_gradient) && is_not_null(m_gradient_region))
  {
    compute_gradient->options().set("regions", std::vector<common::URI>(1, m_gradient_region->uri()));
  }
}

void HeatCouplingRobinHFTB::trigger_setup()
{
  // Get the tags for the used fields
  const std::string temperature_field_tag = options().value<std::string>("temperature_field_tag");
  const std::string temperature_fluid_field_tag = options().value<std::string>("temperature_fluid_field_tag");
  const std::string temperature_solid_field_tag = options().value<std::string>("temperature_solid_field_tag");

  //Handle<AdjacentCellToFace> set_boundary_gradient(get_child("SetBoundaryGradient"));
  Handle<ProtoAction> neumann_heat_flux(get_child("NeumannHeatFlux"));
  Handle<ProtoAction> second_heat_flux(get_child("SecondHeatFlux"));

  // Represents the temperature field, as calculated
  FieldVariable<0, ScalarField> T("Temperature", temperature_field_tag);
  FieldVariable<1, ScalarField> Tfluid("Temperature", temperature_fluid_field_tag);
  FieldVariable<2, ScalarField> Tsolid("Temperature", temperature_solid_field_tag);
  // Represents the gradient of the temperature, to be stored in an (element based) field
  FieldVariable<3, VectorField> GradT("TemperatureGradient", "gradient_field", mesh::LagrangeP0::LibLagrangeP0::library_namespace());

  // to do first of two steps for the Robin BC
  const bool robin_pre = options().value<bool>("robin_pre");

  if (robin_pre == true)
  {
    // Expression for the Robin BC
    neumann_heat_flux->set_expression(elements_expression
    (
     boost::mpl::vector1<mesh::LagrangeP1::Line2D>(), // Valid for surface element types
     group
     (
     _A(T) = _0,
     system_matrix +=  h * (-integral<1>(transpose(N(T))*N(T)*_norm(normal))), // Formulation of Robin Boundary condition
     m_rhs +=  h * (integral<1>(transpose(N(T))*(Tsolid *_norm(normal)))), //-/m_alpha * _norm(GradT * normal)))))         // Tfluid = Tfl * normal - m_alpha * GradT * normal
     _cout << "RobinHFTB_rhs:" << h * (integral<1>(transpose(N(T))*(Tsolid *_norm(normal)))) << "\n"
     )
    ));

    second_heat_flux->set_expression(elements_expression
    (
      boost::mpl::vector2<mesh::LagrangeP0::Line, mesh::LagrangeP1::Line2D>(), // Valid for surface element types
      group(m_rhs(T) += - integral<1>(transpose(N(T))*GradT*normal*m_alpha))
                           //            _cout << "RobinHFTB_rhs_second_flux:" << - integral<1>(transpose(N(T))*GradT*normal*m_alpha) << "\n"
    ));
  }
  else
  {

    // Expression for the Robin BC
    neumann_heat_flux->set_expression(elements_expression
    (
     boost::mpl::vector1<mesh::LagrangeP1::Line2D>(), // Valid for surface element types
     group
     (
     _A(T) = _0,
     system_matrix +=  h * (integral<1>(transpose(N(T))*N(T)*_norm(normal))), // Formulation of Robin Boundary condition
     m_rhs +=  h * (-integral<1>(transpose(N(T))*(Tsolid *_norm(normal)))), //-/m_alpha * _norm(GradT * normal)))))         // Tfluid = Tfl * normal - m_alpha * GradT * normal
     _cout << "RobinHFTB_rhs" << h * (-integral<1>(transpose(N(T))*(Tsolid *_norm(normal)))) << "\n"
     )
    ));

    second_heat_flux->set_expression(elements_expression
    (
      boost::mpl::vector2<mesh::LagrangeP0::Line, mesh::LagrangeP1::Line2D>(), // Valid for surface element types
      group(m_rhs(T) += integral<1>(transpose(N(T))*GradT*normal*m_alpha))
                               //        _cout << "RobinHFTB_rhs_second_flux:" << - integral<1>(transpose(N(T))*GradT*normal*m_alpha)  << "\n"
    ));
  }
  // Raise an event to indicate that we added a variable (GradT)
  common::XML::SignalOptions options;
  common::SignalArgs f = options.create_frame();
  common::Core::instance().event_handler().raise_event("ufem_variables_added", f);
}

} // namespace UFEM

} // namespace cf3
