// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_SignalDispatcher_hpp
#define cf3_common_SignalDispatcher_hpp

/////////////////////////////////////////////////////////////////////////////

#include "common/SignalHandler.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

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

} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_SignalDispatcher_hpp
