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

#include "HeatCouplingRobinFluid.hpp"
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

common::ComponentBuilder < HeatCouplingRobinFluid, common::Action, LibUFEM > HeatCouplingRobinFluid_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

struct ExtractHeatFlux
{
    typedef void result_type;

    template<typename TempT>
    void operator()(TempT& T, Real& flux, const Real& t_bulk) const
    {
        mesh::Mesh& mesh = T.support().mesh();
        mesh::Field& flux_field = Handle<mesh::Dictionary>(mesh.get_child("cf3.mesh.LagrangeP0"))->field("gradient_field");

        const Uint field_begin = flux_field.dict().space(T.support().elements()).connectivity()[0][0];

        const math::VariablesDescriptor& descr = flux_field.descriptor();
        const Uint grad_offset = descr.offset("TemperatureGradient");
        Eigen::Matrix<Real, TempT::EtypeT::dimension, 1> grad_T;
        for(Uint i = 0; i != TempT::EtypeT::dimension; ++i)
        {
            const mesh::Field::ConstRow row = flux_field[field_begin + T.support().element_idx()];
            grad_T[i] = row[grad_offset + i];
        }

        RealVector1 mappedcoord(0.);

        flux = (grad_T.transpose() * T.support().normal(mappedcoord)/(T.support().normal(mappedcoord).norm()))[0];
        std::cout << "flux(from struct):" << flux << "grad_T(from struct):" << grad_T.transpose() << "\n" ;

    }
};

static solver::actions::Proto::MakeSFOp<ExtractHeatFlux>::type const extract_heat_flux = {};

HeatCouplingRobinFluid::HeatCouplingRobinFluid(const std::string& name) :
  Action(name),
  m_rhs(options().add("lss", Handle<math::LSS::System>())
    .pretty_name("LSS")
    .description("The linear system for which the boundary condition is applied")),
  system_matrix(options().option("lss")),
  R_solid("resistance_solid_coefficient"),
  t_bulk("bulk_temperature"),
  m_alpha("scalar_coefficient"),
  lambda_f("thermal_conductivity_fluid"),
  lambda_s("thermal_conductivity_solid"),
  rho("density"),
  cp("specific_heat_capacity")
{
  options().add("temperature_field_tag", UFEM::Tags::solution())
    .pretty_name("Temperature Field Tag")
    .description("Tag for the temperature field in the region where the gradient needs to be calculated")
    .attach_trigger(boost::bind(&HeatCouplingRobinFluid::trigger_setup, this));

  options().add("temperature_fluid_field_tag", UFEM::Tags::solution())
    .pretty_name("Temperature Fluid Field Tag")
    .description("Tag for the ambient temperature field(for the Robin BC)")
    .attach_trigger(boost::bind(&HeatCouplingRobinFluid::trigger_setup, this));

  options().add("gradient_region", m_gradient_region)
    .pretty_name("Gradient Region")
    .description("The (volume) region in which to calculate the temperature gradient")
    .attach_trigger(boost::bind(&HeatCouplingRobinFluid::trigger_gradient_region, this))
    .link_to(&m_gradient_region);

  // Finally set the boundary condition
  create_static_component<ProtoAction>("NeumannHeatFlux");
  create_static_component<ProtoAction>("SecondHeatFlux");

  trigger_setup();
}

HeatCouplingRobinFluid::~HeatCouplingRobinFluid()
{
}

void HeatCouplingRobinFluid::on_regions_set()
{
  get_child("NeumannHeatFlux")->options().set("regions", options()["regions"].value());
  get_child("SecondHeatFlux")->options().set("regions", options()["regions"].value());
}

void HeatCouplingRobinFluid::trigger_gradient_region()
{
  Handle<Component> compute_gradient = get_child("ComputeGradient");
  if(is_not_null(compute_gradient) && is_not_null(m_gradient_region))
  {
    compute_gradient->options().set("regions", std::vector<common::URI>(1, m_gradient_region->uri()));
  }
}

void HeatCouplingRobinFluid::trigger_setup()
{
  // Get the tags for the used fields
  const std::string temperature_field_tag = options().value<std::string>("temperature_field_tag");
  const std::string temperature_fluid_field_tag = options().value<std::string>("temperature_fluid_field_tag");

  //Handle<AdjacentCellToFace> set_boundary_gradient(get_child("SetBoundaryGradient"));
  Handle<ProtoAction> neumann_heat_flux(get_child("NeumannHeatFlux"));
  Handle<ProtoAction> second_heat_flux(get_child("SecondHeatFlux"));

  // Represents the temperature field, as calculated
  FieldVariable<0, ScalarField> T("Temperature", temperature_field_tag);
  FieldVariable<2, ScalarField> Tfluid("Temperature", temperature_fluid_field_tag);
  // Represents the gradient of the temperature, to be stored in an (element based) field
  FieldVariable<3, VectorField> GradT("TemperatureGradient", "gradient_field", mesh::LagrangeP0::LibLagrangeP0::library_namespace());

    // Expression for the Robin BC
    neumann_heat_flux->set_expression(elements_expression
    (
     boost::mpl::vector1<mesh::LagrangeP1::Line2D>(), // Valid for surface element types
     group
     (
     _A(Tfluid) = _0,
     system_matrix +=  (R_solid/(boost::proto::lit(rho)*cp) * (  integral<2>(transpose(N(Tfluid))*N(Tfluid)*_norm(normal)))), // Robin system contribution
     m_rhs += (R_solid/(boost::proto::lit(rho)*cp) * (integral<2>(transpose(N(T))*(T *_norm(normal))))) - (R_solid/(boost::proto::lit(rho)*cp) * (  integral<2>(transpose(N(Tfluid))*Tfluid*_norm(normal)))) // First part of Tsolid calculation and Robin system contribution added to RHS (since we solve for a delta T)
     )
    ));

    second_heat_flux->set_expression(elements_expression
    (
      boost::mpl::vector2<mesh::LagrangeP0::Line, mesh::LagrangeP1::Line2D>(), // Valid for surface element types
      group(m_rhs(T) += - integral<2>(transpose(N(T))*GradT*normal*lambda_s/(boost::proto::lit(rho)*cp)))
    ));

  // Raise an event to indicate that we added a variable (GradT)
  common::XML::SignalOptions options;
  common::SignalArgs f = options.create_frame();
  common::Core::instance().event_handler().raise_event("ufem_variables_added", f);

}

void HeatCouplingRobinFluid::execute(){

    Handle<ProtoAction> neumann_heat_flux(get_child("NeumannHeatFlux"));
    Handle<ProtoAction> second_heat_flux(get_child("SecondHeatFlux"));

    neumann_heat_flux->execute();
    second_heat_flux->execute();

}

} // namespace UFEM

} // namespace cf3
