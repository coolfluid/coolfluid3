// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/URI.hpp"

#include "common/BasicExceptions.hpp"
#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/OptionComponent.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Signal.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"

#include "ActionDirector.hpp"


namespace cf3 {
namespace common {

ComponentBuilder < ActionDirector, Action, LibCommon > ActionDirector_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

ActionDirector::ActionDirector(const std::string& name): Action(name)
{
  options().add_option< OptionArrayT<std::string> >("action_order", std::vector<std::string>())
      ->description("Names of the actions to execute in sequence");

  // signals
  regist_signal( "disable_action" )
    ->connect( boost::bind( &ActionDirector::signal_disable_action, this, _1 ) )
    ->description("Disable the action with the given name")
    ->pretty_name("Disable Action")
    ->signature( boost::bind ( &ActionDirector::signature_disable_action, this, _1) );
}


void ActionDirector::execute()
{
  Option& actions_prop = options().option("action_order");
  std::vector<std::string> actions; actions_prop.put_value(actions);

  BOOST_FOREACH(const std::string& action_name, actions)
  {
    // First check if the child exists
    Handle<Component> child = get_child(action_name);
    if(is_null(child))
      throw SetupError(FromHere(), "No component with name " + action_name + " when executing actions in " + uri().string());

    // If it's a link, ensure the link is valid
    Handle<Component> linked_child(follow_link(child));
    if(is_null(linked_child))
      throw SetupError(FromHere(), "Linked action " + action_name + " points to null in " + uri().string());

    Handle<Action> action(linked_child);
    if(is_null(action))
      throw SetupError(FromHere(), "Component with name " + action_name + " is not an action in " + uri().string());

    CFdebug << "Executing action " << action->uri().string()
      << (action->options().check("regions") ? (" over regions " + action->options().option("regions").value_str()) : "") << CFendl;

    action->execute();
  }
}


ActionDirector& ActionDirector::append(Action& action)
{
  if(is_null(action.parent()))
    throw SetupError(FromHere(), "Attempt to link to non-owned action " + action.name() + " in ActionDirector " + uri().path());
  
  Handle<Component> existing_child = get_child(action.name());
  if(is_null(existing_child))
  {
    create_component<Link>(action.name())->link_to(action);
    on_action_added(action);
  }
  else
  {
    // If a child with the given name existed, check that it corresponds to the supplied action
    Handle<Action> existing_action(existing_child);

    if(is_null(existing_action))
      throw ValueExists(FromHere(), "A component named " + action.name() + " already exists in " + uri().string() + ", but it is not a Action");

    if(existing_action.get() != &action)
      throw ValueExists(FromHere(), "An action named " + action.name() + " already exists in " + uri().string() + ", but it is different from the appended action");
  }

  Option& actions_prop = options().option("action_order");
  std::vector<std::string> actions; actions_prop.put_value(actions);

  actions.push_back(action.name());
  actions_prop.change_value(actions);

  return *this;
}


ActionDirector& ActionDirector::append(const Handle<Action>& action)
{
  return append(*action);
}

ActionDirector& ActionDirector::append(const boost::shared_ptr< Action >& action)
{
  Handle<Component> existing_child = get_child(action->name());
  if(is_null(existing_child))
  {
    add_component(action);
    on_action_added(*action);
  }
  else
  {
    // If a child with the given name existed, check that it corresponds to the supplied action
    Handle<Action> existing_action(existing_child);

    if(is_null(existing_action))
      throw ValueExists(FromHere(), "A component named " + action->name() + " already exists in " + uri().string() + ", but it is not a Action");

    if(existing_action.get() != action.get())
      throw ValueExists(FromHere(), "An action named " + action->name() + " already exists in " + uri().string() + ", but it is different from the appended action");
  }

  Option& actions_prop = options().option("action_order");
  std::vector<std::string> actions; actions_prop.put_value(actions);

  actions.push_back(action->name());
  actions_prop.change_value(actions);

  return *this;
}


void ActionDirector::disable_action(const std::string& name)
{
  Option& actions_prop = options().option("action_order");
  std::vector<std::string> actions; actions_prop.put_value(actions);

  const Uint nb_actions = actions.size();

  std::vector<std::string> new_actions;
  new_actions.reserve(nb_actions);

  for(Uint i = 0; i != nb_actions; ++i)
    if(actions[i] != name)
      new_actions.push_back(actions[i]);

  actions_prop.change_value(new_actions);
}

void ActionDirector::signal_disable_action(SignalArgs& node)
{
  XML::SignalOptions options( node );
  const std::string action = options.value<std::string>( "action_name" );
  disable_action(action);
}

void ActionDirector::signature_disable_action(SignalArgs& node)
{
  XML::SignalOptions options( node );

  // Name of the action to disable
  options.add_option< OptionT<std::string> >( "action_name", std::string() )
      ->description("Action to disable")
      ->pretty_name("Action Name");
}

ActionDirector& operator<<(ActionDirector& action_director, Action& action)
{
  return action_director.append(action);
}

ActionDirector& operator<<(ActionDirector& action_director, const Handle<Action>& action)
{
  return action_director.append(action);
}

ActionDirector& operator<<(ActionDirector& action_director, const boost::shared_ptr<Action>& action)
{
  return action_director.append(action);
}

void ActionDirector::on_action_added(Action& action)
{
}


////////////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
