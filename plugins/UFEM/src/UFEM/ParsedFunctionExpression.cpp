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

#include "solver/Tags.hpp"


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

  options().add(solver::Tags::time(), m_time)
      .pretty_name("Time")
      .description("Time tracking component")
      .attach_trigger(boost::bind(&ParsedFunctionExpression::trigger_time_component, this))
      .mark_basic();
  
  options().option("regions").attach_trigger(boost::bind(&ParsedFunctionExpression::trigger_value, this));
  
  m_function.predefined_values.resize(1, 0.);
  m_function.variables( std::vector<std::string>(1, "t") );
}

ParsedFunctionExpression::~ParsedFunctionExpression()
{
  if(is_not_null(m_time))
  {
    m_time->options().option("current_time").detach_trigger(m_time_trigger_id);
  }
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
  
  vars.push_back("t");

  m_function.variables(vars);
  m_function.functions(functions);
  m_function.parse();
  const Real time = m_function.predefined_values.back();
  m_function.predefined_values.assign(vars.size(), 0.);
  m_function.predefined_values.back() = time;
}

void ParsedFunctionExpression::trigger_time_component()
{
  if(is_not_null(m_time))
  {
    m_time->options().option("current_time").detach_trigger(m_time_trigger_id);
  }

  m_time = options().value< Handle<solver::Time> >(solver::Tags::time());

  if(is_not_null(m_time))
  {
    m_time_trigger_id = m_time->options().option("current_time").attach_trigger_tracked(boost::bind(&ParsedFunctionExpression::trigger_time, this));
  }

  trigger_time();
}

void ParsedFunctionExpression::trigger_time()
{
  cf3_assert(is_not_null(m_time));
  m_function.predefined_values.back() = m_time->options().value<Real>("current_time");
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
