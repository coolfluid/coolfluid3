// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/URI.hpp"

#include "Common/BasicExceptions.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionComponent.hpp"

#include "CActionDirector.hpp"


namespace CF {
namespace Common {

ComponentBuilder < CActionDirector, CAction, LibCommon > CActionDirector_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

CActionDirector::CActionDirector(const std::string& name): CAction(name)
{
  m_options.add_option< OptionArrayT<std::string> >("action_order", std::vector<std::string>())
      ->description("Names of the actions to execute in sequence");
}


void CActionDirector::execute()
{
  Option& actions_prop = option("action_order");
  std::vector<std::string> actions; actions_prop.put_value(actions);

  BOOST_FOREACH(const std::string& action_name, actions)
  {
    // First check if the child exists
    Component::Ptr child = get_child_ptr(action_name);
    if(is_null(child))
      throw SetupError(FromHere(), "No component with name " + action_name + " when executing actions in " + uri().string());

    // If it's a link, ensure the link is valid
    Component::Ptr linked_child = child->follow();
    if(is_null(linked_child))
      throw SetupError(FromHere(), "Linked action " + action_name + " points to null in " + uri().string());

    CAction::Ptr action = boost::dynamic_pointer_cast<CAction>(linked_child);
    if(is_null(action))
      throw SetupError(FromHere(), "Component with name " + action_name + " is not an action in " + uri().string());

    action->execute();
  }
}


CActionDirector& CActionDirector::append(CAction& action)
{
  return append(action.as_ptr<CAction>());
}


CActionDirector& CActionDirector::append(const CAction::Ptr& action)
{
  Component::Ptr existing_child = get_child_ptr(action->name());
  if(is_null(existing_child))
  {
    if(action->has_parent())
    {
      CLink& action_link = create_component<CLink>(action->name());
      action_link.link_to(action);
    }
    else
    {
      add_component(action);
    }
    on_action_added(*action);
  }
  else
  {
    // If a child with the given name existed, check that it corresponds to the supplied action
    CAction::Ptr existing_action = boost::dynamic_pointer_cast<CAction>(existing_child);

    if(is_null(existing_action))
      throw ValueExists(FromHere(), "A component named " + action->name() + " already exists in " + uri().string() + ", but it is not a CAction");

    if(existing_action != action)
      throw ValueExists(FromHere(), "An action named " + action->name() + " already exists in " + uri().string() + ", but it is different from the appended action");
  }

  Option& actions_prop = option("action_order");
  std::vector<std::string> actions; actions_prop.put_value(actions);

  actions.push_back(action->name());
  actions_prop.change_value(actions);

  return *this;
}



CActionDirector& operator<<(CActionDirector& action_director, CAction& action)
{
  return action_director.append(action);
}

CActionDirector& operator<<(CActionDirector& action_director, const CAction::Ptr& action)
{
  return action_director.append(action);
}



void CActionDirector::on_action_added(CAction& action)
{
}


////////////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
