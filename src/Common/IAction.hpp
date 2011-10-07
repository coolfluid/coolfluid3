// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_IAction_hpp
#define CF_Common_IAction_hpp

#include "Common/CommonAPI.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

///////////////////////////////////////////////////////////////////////////////////////


/// Abstract interface for actions. You probably want to use CAction as a base to start from, since that implements IAction as a component
class Common_API IAction {
public: // functions

  virtual ~IAction() {}

  /// Execute the action
  virtual void execute () = 0;
};

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_IAction_hpp
