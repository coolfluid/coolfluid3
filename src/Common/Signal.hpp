// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_Signal_hpp
#define CF_Common_Signal_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/shared_ptr.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/signals2/connection.hpp>

#include "Common/Exception.hpp"

#include "Common/XML/SignalFrame.hpp" // try forward declaration

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown when errors detected while handling signals in SignalHandler
/// @author Tiago Quintino
class Common_API SignalError : public Common::Exception {
public:

  /// Constructor
  SignalError (const Common::CodeLocation& where, const std::string& what);
  
  virtual ~SignalError() throw();

}; // SignalError

////////////////////////////////////////////////////////////////////////////////

/// Class that harbours the types handled by the SignalHandler
/// @author Tiago Quintino
class Common_API Signal {
public:

  // typedefs

  /// Signal underlying type
  typedef boost::signals2::signal< void ( XML::SignalFrame& ) > Type;
  /// Signal connection type
  typedef boost::signals2::connection ConnectionType;
  /// Pointer to Signal
  typedef boost::shared_ptr< Type > TypePtr;
  /// Const Pointer to Signal
  typedef boost::shared_ptr< Type const > TypeCPtr;

  // data

  /// the boost signal object
  TypePtr signal;
  /// pointer to another signal that returns the signature of this signal
  TypePtr signature;
  /// signal description
  std::string description;
  /// signal readable name (used by the GUI). For exemple, if key is
  /// "set_options", readable should be "Set options".
  std::string readable_name;
  /// if @c true, the signal is considered as read-only and might be called
  /// during another signal execution. Default value is @c false.
  bool is_read_only;
  /// if @c true, the signal is hidden from the user's view, but still callable
  bool is_hidden;

}; // Signal

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Signal_hpp
