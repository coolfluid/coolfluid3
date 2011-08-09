// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_SignalDispatcher_hpp
#define CF_Common_SignalDispatcher_hpp

/////////////////////////////////////////////////////////////////////////////

#include "Common/SignalHandler.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////

class URI;

/// API to dispatch a signal call.

/// @author Quentin Gasper

class Common_API SignalDispatcher
{

public:

  void dispatch_empty_signal( const std::string & target, const URI & receiver );

  /// Dispaches the provided signal.

  /// @param receiver The component that will receive the signal.
  /// @param target The signal name.
  /// @param args Signal to dipatch.
  virtual void dispatch_signal( const std::string & target,
                                const URI & receiver,
                                SignalArgs & args ) = 0;

}; // SignalDispatcher

/////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_SignalDispatcher_hpp
