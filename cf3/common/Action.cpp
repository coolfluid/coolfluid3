// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "common/Builder.hpp"
#include "common/Signal.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionList.hpp"
#include "common/Action.hpp"
#include "common/FindComponents.hpp"

#include "common/LibCommon.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////////////////

RegisterComponent<Action,LibCommon> register_action;

Action::Action ( const std::string& name ) : Component(name)
{
  // signals

  regist_signal( "execute" )
    .connect( boost::bind( &Action::signal_execute, this, _1 ) )
    .description("Execute the action")
    .pretty_name("Execute");
}


Action& Action::create_action(const std::string& action_provider,
                                      const std::string& name)
{
  boost::shared_ptr<Action> sub_action = build_component_abstract_type<Action>(action_provider,name);
  add_component(sub_action);
  return *sub_action;
}


void Action::signal_execute ( common::SignalArgs& node )
{
  this->execute();
}

////////////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
