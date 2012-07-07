// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/Option.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"
#include "common/FindComponents.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"


#include "UFEM/ParsedFunctionExpression.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;

common::ComponentBuilder < ParsedFunctionExpression, Action, LibUFEM > ParsedFunctionExpression_Builder;

////////////////////////////////////////////////////////////////////////////////

ParsedFunctionExpression::ParsedFunctionExpression(const std::string& name) : ProtoAction(name)
{
  options().add("value", std::vector<std::string>())
    .pretty_name("Value")
    .description("Array of functions to use. Each function is used as a component in the result vector.")
    .attach_trigger(boost::bind(&ParsedFunctionExpression::trigger_value, this))
    .mark_basic();
  
  options().option("regions").attach_trigger(boost::bind(&ParsedFunctionExpression::trigger_value, this));
}

void ParsedFunctionExpression::trigger_value()
{
  if(m_loop_regions.empty())
    return;
  
  const std::vector<std::string> functions = options().value< std::vector<std::string> >("value");

  const Uint dim = find_parent_component<mesh::Mesh>(*regions().front()).dimension();
  
  // Set variables according to the dimension
  std::vector<std::string> vars;
  if(dim > 0)
    vars.push_back("x");
  if(dim > 1)
    vars.push_back("y");
  if(dim > 2)
    vars.push_back("z");

  m_function.variables(vars);
  m_function.functions(functions);
  m_function.parse();
}

const solver::actions::Proto::ScalarFunction& ParsedFunctionExpression::scalar_function()
{
  if(options().option("value").value< std::vector<std::string> >().size() > 1)
    throw BadValue(FromHere(), "Value option for ParsedFunctionExpression " + uri().path() + " has more than one component, can't use as scalar");
  
  return m_function;
}


////////////////////////////////////////////////////////////////////////////////

} // UFEM
} // cf3
