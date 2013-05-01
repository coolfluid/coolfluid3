// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/URI.hpp"
 

#include "common/OptionArray.hpp"

#include "solver/actions/Loop.hpp"

#include "mesh/Region.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace solver {
namespace actions {

/////////////////////////////////////////////////////////////////////////////////////

Loop::Loop ( const std::string& name ) :
  solver::Action(name)
{
  mark_basic();
}

/////////////////////////////////////////////////////////////////////////////////////

LoopOperation& Loop::create_loop_operation(const std::string action_provider)
{
  // The execuation of operations must be in chronological order,
  // hence they get an alphabetical name
  std::string name = action_provider;
  boost::shared_ptr< LoopOperation > sub_operation = build_component_abstract_type<LoopOperation>(action_provider,name);
  add_component(sub_operation);
  return *sub_operation;
}

/////////////////////////////////////////////////////////////////////////////////////

const LoopOperation& Loop::action(const std::string& name) const
{
  return *get_child(name)->handle<LoopOperation>();
}

/////////////////////////////////////////////////////////////////////////////////////

LoopOperation& Loop::action(const std::string& name)
{
  return *get_child(name)->handle<LoopOperation>();
}

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////
