// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_ActionDirector_hpp
#define cf3_common_ActionDirector_hpp

#include "common/Action.hpp"

#include "LibCommon.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////////////

/// Executes a series of actions, configured through a list of names for the actions to execute
/// actions are passed through the "action_order" option and will be executed in the order they are listed
class Common_API ActionDirector : public Action {

public: // typedefs

  typedef boost::shared_ptr<ActionDirector> Ptr;
  typedef boost::shared_ptr<ActionDirector const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  ActionDirector ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "ActionDirector"; }

  /// Action implementation
  virtual void execute();

  /// Append an action to the back of the list, returning a reference to self (for chaining purposes)
  /// The supplied action is added as a child if it had no parent, otherwise a link is created
  /// If this ActionDirector already has a component or an action with the same name that is different from
  /// the supplied action, an error is raised
  /// If the supplied action was added before, its name is added to the execution list a second time
  ActionDirector& append(Action& action);

  /// Overload taking a shared pointer
  ActionDirector& append(const Action::Ptr& action);
  
  /// Disable the action with the given name
  void disable_action(const std::string& name);
  
  /// Signal for disabling an action
  void signal_disable_action(common::SignalArgs& node);

protected:
  /// Called when an action is added. The default implementation does nothing,
  /// derived classes may override this to complete the configuration of added actions
  /// Only invoked when the action was not already a child of this director.
  virtual void on_action_added(Action& action);
  
private:
  /// Signature for the disable_action signal
  void signature_disable_action(common::SignalArgs& node);
};

/// Allow growing of the list of actions using the shift left operator:
/// director << action1 << action2 << action3
/// Same behavior as append.
ActionDirector& operator<<(ActionDirector& action_director, Action& action);

/// Overload for shared pointers
ActionDirector& operator<<(ActionDirector& action_director, const Action::Ptr& action);

/////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_ActionDirector_hpp
