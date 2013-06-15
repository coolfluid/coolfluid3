// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_LocalDispatcher_hpp
#define cf3_common_LocalDispatcher_hpp

#include "common/SignalDispatcher.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////

/// Dispatches a signal locally.

/// @author Quentin Gasper

class LocalDispatcher : public SignalDispatcher
{
public:

  /// Dispaches the provided signal to the local tree (maintained by the CF Core).

  /// @param receiver The component that will receive the signal.
  /// @param target The signal name.
  /// @param args Signal to dipatch.
  virtual void dispatch_signal(const std::string & target, const URI & receiver,
                               SignalArgs & args);

}; // LocalDispatcher

/////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_LocalDispatcher_hpp
