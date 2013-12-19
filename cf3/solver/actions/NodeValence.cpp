// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Builder.hpp"

#include "mesh/LagrangeP1/ElementTypes.hpp"

#include "Proto/Expression.hpp"

#include "NodeValence.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < NodeValence, common::Action, LibActions > NodeValence_Builder;

///////////////////////////////////////////////////////////////////////////////////////

using namespace Proto;

struct SetNodeValence
{
  typedef void result_type;
  
  template<typename ValenceT>
  void operator()(ValenceT& v) const
  {
    v.add_nodal_values(ValenceT::ElementVectorT::Ones());
  }
};

static MakeSFOp<SetNodeValence>::type const set_node_valence = {};

NodeValence::NodeValence ( const std::string& name ) :
  ProtoAction(name)
{
  FieldVariable<0, ScalarField> valence("Valence", "node_valence");
  
  set_expression(elements_expression(mesh::LagrangeP1::CellTypes(), set_node_valence(valence)));
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

