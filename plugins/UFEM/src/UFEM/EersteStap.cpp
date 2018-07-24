// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <iostream>
#include <string>
#include "common/EigenAssertions.hpp"
#include <Eigen/Dense>

#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include <common/EventHandler.hpp>

#include <vector>
#include <iostream>
// #include "math/LSS/System.hpp"
#include <cmath>


#include "mesh/Region.hpp"
#include "mesh/LagrangeP1/ElementTypes.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"

#include "EersteStap.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;
using namespace std;
using namespace Eigen;
using boost::proto::lit;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < EersteStap, common::ActionDirector, LibUFEM > EersteStap_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

namespace detail
{

struct setresult
{
  typedef void result_type;

  template<typename ResultFieldT, typename NodalNormalT, typename ElementIntegralT>
  void operator()(ResultFieldT& result_field, const NodalNormalT& n, const ElementIntegralT& elint) const
  {
    typename ResultFieldT::ValueT nodal_values;

    for(int i = 0; i != ResultFieldT::ValueT::RowsAtCompileTime; ++i)
    {
      //nodal_values.row(i) = n.value().row(i) * elint.transpose();
      nodal_values.row(i) = elint;
    }

    result_field.add_nodal_values(nodal_values);
  }
};

static MakeSFOp<setresult>::type const set_result = {};

}

EersteStap::EersteStap(const std::string& name) :
  ActionDirector(name),
  lambda_f("thermal_conductivity_fluid"),
  rho("density"),
  cp("specific_heat_capacity")
{

  // options().add("temperature_field_tag", UFEM::Tags::solution())
  //   .pretty_name("Temperature Field Tag")
  //   .description("Tag for the temperature field in the region where the gradient needs to be calculated")
  //   .attach_trigger(boost::bind(&EersteStap::trigger_setup, this));

  create_static_component<ProtoAction>("ZeroFields");
  // Compute the gradient
  create_static_component<ProtoAction>("SensDer");

  trigger_setup();
}

EersteStap::~EersteStap()
{
}

void EersteStap::on_regions_set()
{
  get_child("ZeroFields")->options().set("regions", options()["regions"].value());
  get_child("SensDer")->options().set("regions", options()["regions"].value());
}

void EersteStap::trigger_setup()
{
  Handle<ProtoAction> zero_fields(get_child("ZeroFields"));


  // Get the tags for the used fields
  //const std::string temperature_field_tag = options().value<std::string>("field_tag");
  // static MakeSFOp<setresult>::type const set_result = {};
  Handle<ProtoAction> compute_flux_fluid(get_child("SensDer"));

  // Represents the temperature field, as calculated
  //FieldVariable<0, ScalarField> T("Temperature", temperature_field_tag);
  // Represents the gradient of the temperature, to be stored in an (element based) field

  FieldVariable<0, VectorField> n("NodalNormal", "nodal_normals");
  FieldVariable<1, ScalarField> q("AdjPressure", "adjoint_solution");
  FieldVariable<2, VectorField> grad_Ux("grad_Ux", "Adjvelocity_gradient");
  FieldVariable<3, VectorField> grad_ux("grad_ux", "velocity_gradient");
  FieldVariable<4, ScalarField> J("SensDer", "sensitivity_derivative");//, mesh::LagrangeP0::LibLagrangeP0::library_namespace());
  FieldVariable<5, VectorField> grad_Uy("grad_Uy", "Adjvelocity_gradient");
  FieldVariable<6, VectorField> grad_uy("grad_uy", "velocity_gradient");
  FieldVariable<7, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");


  zero_fields->set_expression(nodes_expression(J = 0.0));
  //set_expression(elements_expression(Vect1) = (grad_Ux[0]*normal[0])+(grad_Ux[1]*normal[1])); // zo definieer je een matrix
  // Hoe dit doen voor grad_ux...

  // Expression to calculate the gradient, at the cell centroid:
  // nabla(T, center) is the shape function gradient matrix evaluated at the element center
  // T are the nodal values for the temperature
  compute_flux_fluid->set_expression(elements_expression
  (
      //boost::mpl::vector<mesh::LagrangeP1::Line2D>(),
    boost::mpl::vector<mesh::LagrangeP1::Line2D>(),

    //J = integral<2> nu_eff*(grad_Ux*normal)//*grad_ux[0]+nu_eff*(grad_Ux[1]*normal[1])*grad_ux[1]-q*normal[0]*grad_ux[0]-q*normal[1]*grad_ux[1]) // Calculate the first part of the sensitivity derivative at the first gauss point, i.e. the cell center
    // uitdrukking van 1ste deel sensitivity: vector(1x2) transpose([grad_Ux[0]*normal[0]+grad_Ux[1]*normal[1],grad_Uy[0]*normal[0]+grad_Uy[1]*normal[1]])
    // Deze vector vermenidgvuldigen met [grad_ux[0] grad_ux[1],  grad_uy[0] grad_uy[1]]
    // inaal wordt de vector (1x2), [(grad_Ux[0]*normal[0]*grad_Ux[1]*normal[1])*grad_ux[0] + (grad_Uy[0]*normal[0]*grad_Uy[1]*normal[1])*grad_uy[0]  ]
    // 2 de deel wordt: [(grad_Ux[0]*normal[0]*grad_Ux[1]*normal[1])*grad_ux[1] + (grad_Uy[0]*normal[0]*grad_Uy[1]*normal[1])*grad_uy[1]  ]

    //detail::set_result(J, n, integral<2>((-(nu*grad_Ux[0]*normal[0]+nu*grad_Ux[1]*normal[1]-q*normal[0])*grad_ux[0] - (nu*grad_Uy[0]*normal[0]+nu*grad_Uy[1]*normal[1]-q*normal[1])*grad_uy[0])*_norm(normal)),
    //integral<2>((-(nu*grad_Ux[0]*normal[0]+nu*grad_Ux[1]*normal[1]-q*normal[0])*grad_ux[1] - (nu*grad_Uy[0]*normal[0]+nu*grad_Uy[1]*n[1]-q*normal[1])*grad_uy[1])*_norm(normal)))

    //detail::set_result(J, n, integral<2>(grad_Ux[0]),integral<2>(grad_Ux[0]))
    // detail::set_result(J, n, integral<2> (transpose(u)*U),integral<2> ((transpose(u)*U)))

    //detail::set_result(J, n, integral<2>(-(nu_eff*(grad_Ux*normal)[0] - q*normal[0])*grad_ux - (nu_eff*(grad_Uy*normal)[0] - q*normal[1])*grad_uy))
    detail::set_result(J, n, integral<2>((-(nu_eff*(grad_Ux*normal)[0] - q*normal[0])*grad_ux - (nu_eff*(grad_Uy*normal)[0] - q*normal[1])*grad_uy)*normal/_norm(normal)))

  ));

}

} // namespace UFEM

} // namespace cf3
