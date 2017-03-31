// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// Dit script is om de Delta(Xk)/Dleta(Bn) te bepalen

#include <cmath>

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Component.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/PropertyList.hpp"

#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"
#include "mesh/LagrangeP1/ElementTypes.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"

#include "solver/actions/Iterate.hpp"
#include "solver/actions/NodeValence.hpp"
#include "solver/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "TweedeStap.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace solver::actions::Proto;
using boost::proto::lit;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < TweedeStap, common::Action, LibUFEM > TweedeStap_builder;

////////////////////////////////////////////////////////////////////////////////////////////

namespace detail
{

struct setnormal
{
  typedef void result_type;

  template<typename NormalT, typename NodalNormalsT>
  void operator()(const NormalT& n, NodalNormalsT& n_out) const
  {
    typename NodalNormalsT::ValueT node_normals;
    node_normals.col(0).setConstant(n[0]);
    node_normals.col(1).setConstant(n[1]);
    n_out.add_nodal_values(node_normals);
  }
};

static MakeSFOp<setnormal>::type const set_normal = {};

}

TweedeStap::TweedeStap(const std::string& name) :
  ProtoAction(name)
{
  m_zero_fields = create_component<ProtoAction>("ZeroFields");

  FieldVariable<0, VectorField> n_out("NodalNormal", "nodal_normals");
  set_expression(elements_expression
  (
    boost::mpl::vector<mesh::LagrangeP1::Line2D>(),
    detail::set_normal(normal(gauss_points_1), n_out)

  ));

  m_zero_fields->set_expression(nodes_expression(group(n_out[_i]=0)));

}

void TweedeStap::execute()
{
  m_zero_fields->execute();
  ProtoAction::execute();
}


void TweedeStap::on_regions_set()
{
  mesh::Field& n = mesh().geometry_fields().create_field("NodalNormal", VECTOR_2D);
  n.add_tag("nodal_normals");
  m_zero_fields->options().set("regions", options().option("regions").value());
}

} // UFEM
} // cf3
