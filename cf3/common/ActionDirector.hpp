// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_ActionDirector_hpp
#define cf3_common_ActionDirector_hpp

#include <set>

#include "common/Action.hpp"

#include "LibCommon.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////////////

/// Executes actions or links to actions that are direct children of this component.
/// Actions can be deactivated through a list of booleans
class Common_API ActionDirector : public Action
{
public: // functions

  /// Contructor
  /// @param name of the component
  ActionDirector ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "ActionDirector"; }

  /// Execute all active child actions
  virtual void execute();
  
protected:
  /// True if the passed action is disabled
  bool is_disabled(const std::string& name);
  
private:
  void trigger_disabled_actions();
  std::set<std::string> m_disabled_actions;
};

/// Add a link to the passed action as a child
ActionDirector& operator<<(ActionDirector& action_director, Action& action);

/// Add a link to the passed action as a child
template<typename ActionT>
ActionDirector& operator<<(ActionDirector& action_director, const Handle<ActionT>& action)
{
  return action_director << *action;
}

/// Take ownership of the passed action, adding it as a child
ActionDirector& operator<<(ActionDirector& action_director, const boost::shared_ptr<Action>& action);

/// Add a link to the passed action as a child
const boost::shared_ptr<ActionDirector>& operator<<(const boost::shared_ptr<ActionDirector>& action_director, Action& action);

/// Add a link to the passed action as a child
template<typename ActionT>
const boost::shared_ptr<ActionDirector>& operator<<(const boost::shared_ptr<ActionDirector>& action_director, const Handle<ActionT>& action)
{
  return action_director << *action;
}

/// Take ownership of the passed action, adding it as a child
const boost::shared_ptr<ActionDirector>& operator<<(const boost::shared_ptr<ActionDirector>& action_director, const boost::shared_ptr<Action>& action);

/////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_ActionDirector_hpp
