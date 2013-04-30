// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_Action_hpp
#define cf3_common_Action_hpp

#include "common/Component.hpp"
#include "common/IAction.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

///////////////////////////////////////////////////////////////////////////////////////

/// Component that executes an action. Implementation of the IAction interface as a component, exposing the execute function as a signal.
class Common_API Action : public IAction, public Component {

public: // functions

  /// Contructor
  /// @param name of the component
  Action ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "Action"; }

  /// execute the action
  virtual void execute () = 0;

  /// create an action inside this action
  /// @deprecated should use create_component()
  virtual Action& create_action(const std::string& action_provider, const std::string& name);

  /// @name SIGNALS
  //@{

  /// signal to execute this action
  void signal_execute ( common::SignalArgs& node );

  //@} END SIGNALS

};

/////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_Action_hpp
