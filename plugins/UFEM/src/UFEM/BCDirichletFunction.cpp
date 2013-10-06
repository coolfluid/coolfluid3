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

#include "BCDirichletFunction.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/Expression.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;
using boost::proto::lit;

common::ComponentBuilder < BCDirichletFunction, common::Action, LibUFEM > BCDirichletFunction_Builder;

BCDirichletFunction::BCDirichletFunction(const std::string& name) :
  ParsedFunctionExpression(name),
  m_dirichlet(options().add("lss", Handle<math::LSS::System>()).pretty_name("LSS").description("The linear system for which the boundary condition is applied"))
{
  options().add("variable_name", "variable_name-NOT_SET")
    .pretty_name("Variable Name")
    .description("Name of the variable for which to set the BC")
    .mark_basic();
    
  options().add("field_tag", "field_tag-NOT_SET")
    .pretty_name("Field Tag")
    .description("Tag of the field to which the variable belongs")
    .mark_basic();
    
  options().add("space", "geometry")
    .pretty_name("Space")
    .description("Name of the space to use, for example: cf3.mesh.LagrangeP2. Defaults to whatever geometry space is used.")
    .mark_basic();

  options().add("solving_for_difference", true)
    .pretty_name("Solving for Difference")
    .description("Indicate if we are solving for the difference between two time steps")
    .mark_basic();
}

BCDirichletFunction::~BCDirichletFunction()
{
}

void BCDirichletFunction::execute()
{
  if(m_loop_regions.empty())
  {
    CFwarn << "No regions set for " << uri().path() << CFendl;
    return;
  }
  
  cf3_assert(is_not_null(options().value< Handle<math::LSS::System> >("lss")));
  
  if(!expression_is_set())
  {
    const Uint dim = common::find_parent_component<mesh::Mesh>(*regions().front()).dimension();
    if(vector_function().nbfuncs() == 1)
    {
      FieldVariable<0, ScalarField> var(options().value<std::string>("variable_name"), options().value<std::string>("field_tag"), options().value<std::string>("space"));
      if(options().value<bool>("solving_for_difference"))
        set_expression(nodes_expression(m_dirichlet(var)  = scalar_function()));
      else
        set_expression(nodes_expression(m_dirichlet(var)  = scalar_function() + var));
    }
    else if(vector_function().nbfuncs() == dim)
    {
      FieldVariable<0, VectorField> var(options().value<std::string>("variable_name"), options().value<std::string>("field_tag"), options().value<std::string>("space"));
      if(options().value<bool>("solving_for_difference"))
        set_expression(nodes_expression(m_dirichlet(var)  = vector_function()));
      else
        set_expression(nodes_expression(m_dirichlet(var)  = vector_function() + var));
    }
  }
  
  cf3::solver::actions::Proto::ProtoAction::execute();
}

} // namespace UFEM
} // namespace cf3
