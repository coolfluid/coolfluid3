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
  
  Handle<ProtoAction> zero_field = create_static_component<ProtoAction>("ZeroField");
  zero_field->set_expression(nodes_expression(valence = 0.));
  m_zero_field = zero_field;
  
  set_expression(elements_expression(mesh::LagrangeP1::CellTypes(), set_node_valence(valence)));
}

void NodeValence::on_regions_set()
{
  cf3::solver::Action::on_regions_set();
  m_zero_field->options().set("regions", options().option("regions").value());
}

void NodeValence::execute()
{
  m_zero_field->execute();
  cf3::solver::actions::Proto::ProtoAction::execute();
}


////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

