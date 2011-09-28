// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CAction_hpp
#define CF_Common_CAction_hpp

#include "Common/Component.hpp"
#include "Common/IAction.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

///////////////////////////////////////////////////////////////////////////////////////

/// Component that executes an action. Implementation of the IAction interface as a component, exposing the execute function as a signal.
class Common_API CAction : public IAction, public Component {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CAction> Ptr;
  typedef boost::shared_ptr<CAction const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CAction ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "CAction"; }

  /// execute the action
  virtual void execute () = 0;

  /// create an action inside this action
  /// @deprecated should use create_component()
  virtual CAction& create_action(const std::string& action_provider, const std::string& name);

  /// @name SIGNALS
  //@{

  /// signal to execute this action
  void signal_execute ( Common::SignalArgs& node );

  //@} END SIGNALS

};

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CAction_hpp
