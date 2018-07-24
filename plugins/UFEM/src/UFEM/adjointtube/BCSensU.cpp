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
#include <cmath>
#include "mesh/Region.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "solver/actions/Proto/ElementGradDiv.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"
#include "solver/actions/Proto/Partial.hpp"

#include "BCSensU.hpp"
#include "../AdjacentCellToFace.hpp"
#include "../Tags.hpp"

#include "solver/actions/Proto/Expression.hpp"

namespace cf3
{

namespace UFEM
{
namespace adjointtube
{
using namespace solver::actions::Proto;
using boost::proto::lit;

common::ComponentBuilder < BCSensU, common::Action, LibUFEMAdjointTube > BCSensU_Builder;

BCSensU::BCSensU(const std::string& name) :
  ProtoAction(name),
  m_dirichlet(options().add("lss", Handle<math::LSS::System>()).pretty_name("LSS").description("The linear system for which the boundary condition is applied"))
{
  FieldVariable<0, VectorField> grad_ux("grad_ux", "velocity_gradient");
  FieldVariable<1, VectorField> SensU("SensU", "sensitivity_solution");
  FieldVariable<2, VectorField> grad_uy("grad_uy", "velocity_gradient");
  FieldVariable<3, VectorField> n("NodalNormal", "nodal_normals");
  FieldVariable<4, ScalarField> node_filter("node_filter", "node_filter");


  set_expression(nodes_expression(group
  (
    m_dirichlet(SensU[0])  = -(grad_ux[0]*n[0] + grad_ux[1]*n[1])*node_filter,
    m_dirichlet(SensU[1])  = -(grad_uy[0]*n[0] + grad_uy[1]*n[1])*node_filter
  )));

  // FieldVariable<0, VectorField> grad_ux2x("grad_ux2x", "ux_hessian");
  // FieldVariable<1, VectorField> grad_ux2y("grad_ux2y", "ux_hessian");
  // FieldVariable<2, VectorField> grad_uy2x("grad_uy2x", "uy_hessian");
  // FieldVariable<3, VectorField> grad_uy2y("grad_uy2y", "uy_hessian");
  // FieldVariable<4, ScalarField> SensP("SensP", "sensitivity_solution");
  // FieldVariable<5, ScalarField> node_filter("node_filter", "node_filter");
  // FieldVariable<6, VectorField> SensU("SensU", "sensitivity_solution");
  // FieldVariable<7, VectorField> n("NodalNormal", "nodal_normals");

  // set_expression(elements_expression
  // (
  //   boost::mpl::vector1 <mesh::LagrangeP1::Line2D>(), // Valid for surface element types
  //   group
  //   (
  //     _A(SensU) = _0, _A(SensP) = _0, _a[SensU] = _0, _a[SensP] = _0,
  //     element_quadrature
  //     (
  //       _a[SensU[0]] += transpose(N(SensU)) * ((grad_ux2x*normal)[0]*n[0] + (grad_ux2y*normal)[0]*n[1]) * node_filter,
  //       _a[SensU[1]] += transpose(N(SensU)) * ((grad_uy2x*normal)[0]*n[0] + (grad_uy2y*normal)[0]*n[1]) * node_filter
  //     ),
  //     m_rhs += _a
  //   )
  // ));
}

BCSensU::~BCSensU()
{
}


} // namespace adjointtube
} // namespace UFEM
} // namespace cf3
