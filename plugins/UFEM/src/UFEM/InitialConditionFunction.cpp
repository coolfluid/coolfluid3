// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"

#include "common/XML/SignalOptions.hpp"

#include "math/VariableManager.hpp"
#include "math/VariablesDescriptor.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"

#include "physics/PhysModel.hpp"

#include "solver/Tags.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "InitialConditionFunction.hpp"
#include "ParsedFunctionExpression.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace common::XML;
using namespace math;
using namespace mesh;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < InitialConditionFunction, common::Action, LibUFEM > InitialConditionFunction_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

InitialConditionFunction::InitialConditionFunction(const std::string& name) : ParsedFunctionExpression(name)
{
  options().add("variable_name", "")
    .pretty_name("Variable Name")
    .description("Name for the variable to set")
    .attach_trigger(boost::bind(&InitialConditionFunction::trigger, this))
    .mark_basic();

  options().add("field_tag", "")
    .pretty_name("Field Tag")
    .description("Tag for the field in which the initial conditions will be set")
    .attach_trigger(boost::bind(&InitialConditionFunction::trigger, this));
    
  options().add("field_space_name", "geometry")
    .pretty_name("Field Space Name")
    .description("Name of the space used by the field in the initial condition")
    .attach_trigger(boost::bind(&InitialConditionFunction::trigger, this));
}

InitialConditionFunction::~InitialConditionFunction()
{
}


void InitialConditionFunction::trigger()
{
  const std::string field_tag = options().value<std::string>("field_tag");
  const std::string var_name = options().value<std::string>("variable_name");
  const std::string field_space_name = options().value<std::string>("field_space_name");
  if(field_tag.empty() || var_name.empty())
    return;

  VariablesDescriptor& descriptor = find_component_with_tag<VariablesDescriptor>(physical_model().variable_manager(), field_tag);

  if(!descriptor.has_variable(var_name))
    throw SetupError(FromHere(), "No variable named " + var_name + " found in field with tag " + field_tag + " in setup of " + uri().path());

  if(descriptor.var_length(var_name) == 1)
  {
    FieldVariable<0, ScalarField> v(var_name, field_tag, field_space_name);
    set_expression(nodes_expression(v = scalar_function()));
  }
  else
  {
    FieldVariable<0, VectorField> v(var_name, field_tag, field_space_name);
    set_expression(nodes_expression(v = vector_function()));
  }
}



} // UFEM
} // cf3
