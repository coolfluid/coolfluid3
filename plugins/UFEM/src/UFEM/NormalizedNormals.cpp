// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

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

#include "solver/actions/Iterate.hpp"
#include "solver/actions/NodeValence.hpp"
#include "solver/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "NormalizedNormals.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace solver::actions::Proto;
using boost::proto::lit;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < NormalizedNormals, common::Action, LibUFEM > NormalizedNormals_builder;

////////////////////////////////////////////////////////////////////////////////////////////

namespace detail
{

struct SetNormals
{
  typedef void result_type;

  template<typename NormalT, typename NodalNormalsT>
  void operator()(const NormalT& n, NodalNormalsT& n_out) const
  {
    typename NodalNormalsT::ValueT node_normals;
    for(int column = 0; column != NodalNormalsT::ValueT::ColsAtCompileTime; ++column)
    {
      node_normals.col(column).setConstant(n[column]);
    }
    n_out.add_nodal_values(node_normals);
  }
};

static MakeSFOp<SetNormals>::type const set_normal = {};

}

NormalizedNormals::NormalizedNormals(const std::string& name) :
  ProtoAction(name)
{
  m_zero_fields = create_component<ProtoAction>("ZeroFields");
  m_normalize = create_component<ProtoAction>("Normalize");

  FieldVariable<0, VectorField> n_out("NodalNormal", "nodal_normals");

  set_expression(elements_expression
  (
    boost::mpl::vector<mesh::LagrangeP1::Line2D>(),
    detail::set_normal(normal(gauss_points_1), n_out)
  ));

  m_zero_fields->set_expression(nodes_expression(n_out[_i]=0));
  m_normalize->set_expression(nodes_expression(n_out = n_out / _norm(n_out)));
}

void NormalizedNormals::execute()
{
  m_zero_fields->execute();
  ProtoAction::execute();
  m_normalize->execute();
}


void NormalizedNormals::on_regions_set()
{
  const Uint dim = physical_model().ndim();
  mesh::Field& n = mesh().geometry_fields().create_field("NodalNormal", dim == 3 ? VECTOR_3D : (dim == 2 ? VECTOR_2D : SCALAR));
  n.add_tag("nodal_normals");

  m_zero_fields->options().set("regions", options().option("regions").value());
  m_normalize->options().set("regions", options().option("regions").value());
}

} // UFEM
} // cf3
