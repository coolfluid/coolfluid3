// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/Option.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"


#include "UFEM/ParsedFunctionExpression.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;

common::ComponentBuilder < ParsedFunctionExpression, Action, LibUFEM > ParsedFunctionExpression_Builder;

////////////////////////////////////////////////////////////////////////////////

ParsedFunctionExpression::ParsedFunctionExpression(const std::string& name) : ProtoAction(name)
{
  options().add_option("value", std::vector<std::string>())
    .pretty_name("Value")
    .description("Array of functions to use. Each function is used as a component in the result vector.")
    .attach_trigger(boost::bind(&ParsedFunctionExpression::trigger_value, this));
}

void ParsedFunctionExpression::trigger_value()
{
  const std::vector<std::string> functions = options().option("value").value< std::vector<std::string> >();

  // Set variables according to the dimension
  std::vector<std::string> vars;
  if(functions.size() > 0)
    vars.push_back("x");
  if(functions.size() > 1)
    vars.push_back("y");
  if(functions.size() > 2)
    vars.push_back("z");

  m_function.variables(vars);
  m_function.functions(functions);
  m_function.parse();
}

////////////////////////////////////////////////////////////////////////////////

} // UFEM
} // cf3
