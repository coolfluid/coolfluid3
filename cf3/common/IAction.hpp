// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_IAction_hpp
#define cf3_common_IAction_hpp

#include "common/CommonAPI.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

///////////////////////////////////////////////////////////////////////////////////////


/// Abstract interface for actions. You probably want to use Action as a base to start from, since that implements IAction as a component
class Common_API IAction {
public: // functions

  virtual ~IAction() {}

  /// Execute the action
  virtual void execute () = 0;
};

/////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_IAction_hpp
