// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CActionDirector_hpp
#define CF_Common_CActionDirector_hpp

#include "Common/CAction.hpp"

#include "LibCommon.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

/// Executes a series of actions, configured through a list of names for the actions to execute
/// Actions are passed through the "ActionList" option and will be executed in the order they are listed
class Common_API CActionDirector : public CAction
{
public: // typedefs

  typedef boost::shared_ptr<CActionDirector> Ptr;
  typedef boost::shared_ptr<CActionDirector const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CActionDirector ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "CActionDirector"; }
  
  /// CAction implementation
  virtual void execute();
  
  /// Append an action to the back of the list, returning a reference to self (for chaining purposes)
  /// The supplied action is added as a child if it had no parent, otherwise a link is created
  /// If this CActionDirector already has a component or an action with the same name that is different from
  /// the supplied action, an error is raised
  /// If the supplied action was added before, its name is added to the execution list a second time
  CActionDirector& append(CAction& action);
  
  /// Overload taking a shared pointer
  CActionDirector& append(const CAction::Ptr& action);
  
protected:
  /// Called when an action is added. The default implementation does nothing,
  /// derived classes may override this to complete the configuration of added actions
  /// Only invoked when the action was not already a child of this director.
  virtual void on_action_added(CAction& action);
};

/// Allow growing of the list of actions using the shift left operator:
/// director << action1 << action2 << action3
/// Same behavior as append.
CActionDirector& operator<<(CActionDirector& action_director, CAction& action);

/// Overload for shared pointers
CActionDirector& operator<<(CActionDirector& action_director, const CAction::Ptr& action);

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CActionDirector_hpp
