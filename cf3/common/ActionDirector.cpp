// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/BasicExceptions.hpp"
#include "common/Builder.hpp"
#include "common/ComponentIterator.hpp"
#include "common/Log.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/OptionComponent.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Signal.hpp"
#include "common/URI.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"

#include "ActionDirector.hpp"

namespace cf3 {
namespace common {

ComponentBuilder < ActionDirector, Action, LibCommon > ActionDirector_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

ActionDirector::ActionDirector(const std::string& name): Action(name)
{
  options().add("disabled_actions", std::vector<std::string>())
    .description("Names of the actions to disable")
    .pretty_name("Disabled Actions")
    .attach_trigger(boost::bind(&ActionDirector::trigger_disabled_actions, this));
}

void ActionDirector::execute()
{
  BOOST_FOREACH(Component& child, *this)
  {
    Handle<Action> action(follow_link(child));
    
    const bool disabled = is_not_null(action) ? is_disabled(action->name()) : true;
    if(!disabled)
    {
      CFdebug << name() << ": Executing action " << action->uri().path() << CFendl;
      action->execute();
    }
    else
    {
      if(is_not_null(action))
        CFdebug << name() << ": Skipping disabled action " << action->uri().path() << CFendl;
      else
        CFdebug << name() << ": Doing nothing for non-action " << child.uri().path() << CFendl;
    }
  }
}

bool ActionDirector::is_disabled(const std::string& name)
{
  return m_disabled_actions.count(name);
}

void ActionDirector::trigger_disabled_actions()
{
  m_disabled_actions.clear();

  std::vector<std::string> disabled_actions = options().value< std::vector<std::string> >("disabled_actions");
  m_disabled_actions.insert(disabled_actions.begin(), disabled_actions.end());
}

ActionDirector& operator<<(ActionDirector& action_director, Action& action)
{
  action_director.add_link(action);
  return action_director;
}

ActionDirector& operator<<(ActionDirector& action_director, const boost::shared_ptr<Action>& action)
{
  action_director.add_component(action);
  return action_director;
}

const boost::shared_ptr< ActionDirector >& operator<<(const boost::shared_ptr< ActionDirector >& action_director, Action& action)
{
  action_director->add_link(action);
  return action_director;
}

const boost::shared_ptr< ActionDirector >& operator<<(const boost::shared_ptr< ActionDirector >& action_director, const boost::shared_ptr< Action >& action)
{
  action_director->add_component(action);
  return action_director;
}


////////////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
