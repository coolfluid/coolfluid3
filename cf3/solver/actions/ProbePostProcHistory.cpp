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

#include "solver/History.hpp"
#include "solver/actions/ProbePostProcHistory.hpp"

namespace cf3 {
namespace solver {
namespace actions {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder <ProbePostProcHistory,ProbePostProcessor, solver::actions::LibActions> ProbePostProcHistory_builder;

////////////////////////////////////////////////////////////////////////////////

ProbePostProcHistory::ProbePostProcHistory(const std::string &name) : ProbePostProcessor(name)
{
  options().add("history",m_history)
      .description("history")
      .link_to(&m_history);
  options().add("variables",m_vars)
      .description("Variables to log in the history component")
      .link_to(&m_vars)
      .mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void ProbePostProcHistory::execute()
{
  boost_foreach( const std::string& var_name, m_vars )
  {
    if (!m_probe->variables()->has_variable(var_name))
      throw SetupError(FromHere(),"Variable "+var_name+" not found in "+m_probe->uri().string());

    m_history->set(m_probe->name()+"_"+var_name,m_probe->properties().value<Real>(var_name));
  }
  m_history->save_entry();
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3
