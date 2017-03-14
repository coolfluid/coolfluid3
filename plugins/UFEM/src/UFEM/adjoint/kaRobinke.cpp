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

#include "kaRobinke.hpp"
#include "../AdjacentCellToFace.hpp"
#include "../Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"


namespace cf3
{

namespace UFEM
{
namespace adjoint
{
using namespace solver::actions::Proto;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < kaRobinke, common::Action, LibUFEMAdjoint > kaRobinke_Builder;

////////////////////////////////////////////////////////////////////////////////////////////




kaRobinke::kaRobinke(const std::string& name) :
  Action(name),
  system_rhs(options().add("lss", Handle<math::LSS::System>())
    .pretty_name("LSS")
    .description("The linear system for which the boundary condition is applied")),
  system_matrix(options().option("lss"))

{




    //Handle<AdjacentCellToFace> set_boundary_gradient(get_child("SetBoundaryGradient"));

    auto robincondition = create_static_component<ProtoAction>("Robincondition");

    // Represents the temperature field, as calculated
    FieldVariable<0, ScalarField> ka("ka","keAdjoint_solution");
    FieldVariable<4, ScalarField> epsilona("epsilona", "keAdjoint_solution");
    FieldVariable<1, VectorField> u("Velocity", "navier_stokes_solution");
    FieldVariable<2, ScalarField> epsilon("epsilon","ke_solution");
    FieldVariable<3, ScalarField> k("k","ke_solution");

      // Expression for the Robin BC
      robincondition->set_expression(elements_expression
      (
       boost::mpl::vector2<mesh::LagrangeP1::Line2D,
                           mesh::LagrangeP1::Triag3D
                           >(), // Valid for surface element types
       group
       (
       _A = _0,
       _A(ka,ka) =  integral<2>(transpose(N(ka))*N(ka)*(transpose(u)*transpose(normal))/(m_c_mu*k*k/m_sigma_k/epsilon)),
       // (transpose(u)*transpose(normal))/(nu_eff*grad_Ux[1])
       _A(epsilona, epsilona) = integral<2>(transpose(N(epsilona))*N(epsilona)*(transpose(u)*transpose(normal))/(m_c_mu*k*k/m_sigma_epsilon/epsilon)),

              system_matrix+=_A,
              system_rhs += -_A*_x
      )));


}

kaRobinke::~kaRobinke()
{
}


void kaRobinke::on_regions_set()
{
  get_child("Robincondition")->options().set("regions", options()["regions"].value());

}




void kaRobinke::execute(){

    Handle<ProtoAction> robincondition(get_child("Robincondition"));
    robincondition->execute();

}
} // namespace adjoint
} // namespace UFEM

} // namespace cf3
