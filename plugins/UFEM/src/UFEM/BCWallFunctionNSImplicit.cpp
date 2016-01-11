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

#include "BCWallFunctionNSImplicit.hpp"
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

common::ComponentBuilder < BCWallFunctionNSImplicit, common::Action, LibUFEM > BCWallFunctionNSImplicit_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

using boost::proto::lit;

BCWallFunctionNSImplicit::BCWallFunctionNSImplicit(const std::string& name) :
  Action(name),
  rhs(options().add("lss", Handle<math::LSS::System>())
    .pretty_name("LSS")
    .description("The linear system for which the boundary condition is applied")),
  system_matrix(options().option("lss"))
{
  //create_static_component<ProtoAction>("ZeroBoudaryRows")->options().option("regions").add_tag("norecurse");
  create_static_component<ProtoAction>("NoPenetration")->options().option("regions").add_tag("norecurse");

  trigger_setup();
}

BCWallFunctionNSImplicit::~BCWallFunctionNSImplicit()
{
}


void BCWallFunctionNSImplicit::on_regions_set()
{
  //get_child("ZeroBoudaryRows")->options().set("regions", options()["regions"].value());
  get_child("NoPenetration")->options().set("regions", options()["regions"].value());
}

void BCWallFunctionNSImplicit::trigger_setup()
{

  //Handle<ProtoAction> zero_boundary_rows(get_child("ZeroBoudaryRows"));
  Handle<ProtoAction> no_penetration(get_child("NoPenetration"));

  FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
  FieldVariable<1, ScalarField> p("Pressure", "navier_stokes_solution");
  FieldVariable<2, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");

  // Zero the rows for the continuity equation
  //zero_boundary_rows->set_expression(nodes_expression(zero_row(system_matrix, p)));

  // Set normal component to zero
  no_penetration->set_expression(elements_expression
  (
    boost::mpl::vector1<mesh::LagrangeP1::Line2D>(), // Valid for surface element types
    group
    (
      _A(u) = _0, _A(p) = _0,
      element_quadrature
      (
        _A(p, u[_i]) += -transpose(N(p)) * N(u) * normal[_i]
      ),
      system_matrix +=  _A,
      rhs += -_A * _x
    )
  ));
}

void BCWallFunctionNSImplicit::execute()
{
  //Handle<ProtoAction> zero_boundary_rows(get_child("ZeroBoudaryRows"));
  Handle<ProtoAction> no_penetration(get_child("NoPenetration"));

  //zero_boundary_rows->execute();
  no_penetration->execute();
}

} // namespace UFEM

} // namespace cf3
