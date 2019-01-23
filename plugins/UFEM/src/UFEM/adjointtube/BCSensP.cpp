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

#include "BCSensP.hpp"
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

common::ComponentBuilder < BCSensP, common::Action, LibUFEMAdjointTube > BCSensP_Builder;

namespace detail
{

struct FilterResult
{
  typedef void result_type;

  template<typename ResultT, typename NodeFilterT>
  void operator()(ResultT& result, const NodeFilterT& node_filter) const
  {
    constexpr int nb_nodes = NodeFilterT::EtypeT::nb_nodes;
    if(node_filter.value().sum() < 0.99)
    {
      result.setZero();
    }
    // for(int i = 0; i != nb_nodes; ++i)
    // {
    //   result[i] *= node_filter.value()[i];
    // }
  }
};

static MakeSFOp<FilterResult>::type const filter_result = {};

}

BCSensP::BCSensP(const std::string& name) :
  ProtoAction(name),
  m_rhs(options().add("lss", Handle<math::LSS::System>()).pretty_name("LSS").description("The linear system for which the boundary condition is applied"))
{
  FieldVariable<0, VectorField> grad_p2x("grad_p2x", "pressure_hessian");
  FieldVariable<1, VectorField> grad_p2y("grad_p2y", "pressure_hessian");
  FieldVariable<2, ScalarField> SensP("SensP", "sensitivity_solution");
  FieldVariable<3, ScalarField> node_filter("node_filter", "node_filter");
  FieldVariable<4, VectorField> SensU("SensU", "sensitivity_solution");
  FieldVariable<5, VectorField> n("NodalNormal", "nodal_normals");

  set_expression(elements_expression
  (
    boost::mpl::vector1 <mesh::LagrangeP1::Line2D>(), // Valid for surface element types
    group
    (
      _A(SensU) = _0, _A(SensP) = _0, _a[SensU] = _0, _a[SensP] = _0,
      element_quadrature
      (
        _a[SensP] += transpose(N(SensP)) * ((grad_p2x*transpose(n))[0]*normal[0] + (grad_p2y*transpose(n))[0]*normal[1]) * node_filter
      ),
      m_rhs += _a
    )
  ));

  // FieldVariable<0, VectorField> grad_p("grad_p", "pressure_gradient");
  // FieldVariable<1, ScalarField> SensP("SensP", "sensitivity_solution");
  // FieldVariable<2, ScalarField> node_filter("node_filter", "node_filter");
  // FieldVariable<3, VectorField> n("NodalNormal", "nodal_normals");

  // set_expression(nodes_expression(group
  // (
  //   m_dirichlet(SensP)  = -(grad_p[0]*n[0] + grad_p[1]*n[1])*node_filter
  // )));


}

BCSensP::~BCSensP()
{
}


} // namespace adjointtube
} // namespace UFEM
} // namespace cf3
