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

/// Executes a series of actions, configured through a list of URIs to the actions to execute
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
  CActionDirector& append(const CAction& action);

private:

  /// Called when the list of actions is configured
  /// This caches pointers to the actions, so the URIs must be valid
  void trigger_actions();
  
  /// list with pointers to the actions to execute
  std::vector< boost::weak_ptr<CAction> > m_actions;
};

/// Allow growing of the list of actions using the shift left operator:
/// director << action1 << action2 << action3
CActionDirector& operator<<(CActionDirector& action_director, const CAction& action);

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CActionDirector_hpp
