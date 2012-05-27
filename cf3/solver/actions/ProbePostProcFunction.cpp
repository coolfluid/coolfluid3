// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/algorithm/string.hpp>

#include "common/Core.hpp"
#include "common/Builder.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"
#include "common/FindComponents.hpp"


#include "math/VariablesDescriptor.hpp"

#include "solver/actions/ProbePostProcFunction.hpp"

namespace cf3 {
namespace solver {
namespace actions {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder <ProbePostProcFunction, ProbePostProcessor, LibActions> ProbePostProcFunction_builder;

////////////////////////////////////////////////////////////////////////////////

ProbePostProcFunction::ProbePostProcFunction(const std::string &name) : ProbePostProcessor(name)
{
  m_var_str="f";
  m_function_str="0";
  options().add("function",m_var_str+"="+m_function_str)
      .description("Function to parse. Variables will be added dynamically during probing");
}

////////////////////////////////////////////////////////////////////////////////

void ProbePostProcFunction::execute()
{
  update_function();
  update_params();
  set(m_var_str,function(m_params));
}

////////////////////////////////////////////////////////////////////////////////

void ProbePostProcFunction::update_function()
{
  std::string str = options().value<std::string>("function");
  std::vector<std::string> split;
  boost::split(split, str, boost::is_any_of("="));

  m_var_str = split[0];
  m_function_str = split[1];

  std::stringstream vars;
  // Note, we have to remove "[" and "]" characters from variable names
  for (Uint var_nb=0; var_nb<m_probe->variables()->nb_vars(); ++var_nb)
  {
    if (var_nb != 0) vars << ",";
    std::string var_name = m_probe->variables()->user_variable_name(var_nb);
    std::string mod_var_name(var_name);
    boost::algorithm::replace_all(mod_var_name,"[","_");
    boost::algorithm::replace_all(mod_var_name,"]","_");
    vars << mod_var_name;

    boost::algorithm::replace_all(m_function_str,var_name,mod_var_name);
  }
  function.parse(m_function_str, vars.str());
}

////////////////////////////////////////////////////////////////////////////////

void ProbePostProcFunction::update_params()
{
  m_params.resize(m_probe->variables()->size());
  for (Uint var_nb=0; var_nb<m_probe->variables()->nb_vars(); ++var_nb)
  {
    m_params[var_nb] = m_probe->properties().value<Real>(m_probe->variables()->user_variable_name(var_nb));
  }
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3
