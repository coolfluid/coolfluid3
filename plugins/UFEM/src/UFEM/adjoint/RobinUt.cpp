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
#include <vector>
#include <iostream>
#include "math/LSS/System.hpp"

#include "mesh/Region.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"

#include "RobinUt.hpp"
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

common::ComponentBuilder < RobinUt, common::Action, LibUFEMAdjoint > RobinUt_Builder;

////////////////////////////////////////////////////////////////////////////////////////////




RobinUt::RobinUt(const std::string& name) :
  Action(name),
  system_rhs(options().add("lss", Handle<math::LSS::System>())
    .pretty_name("LSS")
    .description("The linear system for which the boundary condition is applied")),
  system_matrix(options().option("lss"))

{


   const auto u_complement_normal = make_lambda([](const RealVector& u, const RealVector& n)
   {
    if(u.size() == 3)
    {
     return ((u[0]*(1-n[0]))+(u[1]*(1-n[1]))+(u[2]*(1-n[2])));
     }
      return ((u[0]*(1-n[0]))+(u[1]*(1-n[1])));

    });

    //Handle<AdjacentCellToFace> set_boundary_gradient(get_child("SetBoundaryGradient"));

    auto robincondition = create_static_component<ProtoAction>("Robincondition");

    // Represents the temperature field, as calculated
    FieldVariable<0, VectorField> U("AdjVelocity","adjoint_solution");
    FieldVariable<1, VectorField> u("Velocity", "navier_stokes_solution");
    FieldVariable<2, ScalarField> nu_eff("EffectiveViscosity","navier_stokes_viscosity");
    FieldVariable<3, ScalarField> q("AdjPressure","adjoint_solution");

        robincondition->set_expression(elements_expression
        (
         boost::mpl::vector2<mesh::LagrangeP1::Line2D,
                             mesh::LagrangeP1::Triag3D
                             >(), // Valid for surface element types
         group
         (
         _A(U) = _0, _A(q) = _0,
         _A(U[_i],U[_i]) =  integral<2>(transpose(N(U))*N(U)*u_complement_normal(u, normal)/nu_eff),

                system_matrix+=_A,
                system_rhs += -_A*_x
        )));



}

RobinUt::~RobinUt()
{
}


void RobinUt::on_regions_set()
{
  get_child("Robincondition")->options().set("regions", options()["regions"].value());

}




void RobinUt::execute(){

    Handle<ProtoAction> robincondition(get_child("Robincondition"));
    robincondition->execute();

}
} // namespace adjoint
} // namespace UFEM

} // namespace cf3
