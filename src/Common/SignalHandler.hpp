// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_SignalHandler_hpp
#define CF_Common_SignalHandler_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

namespace XML { class SignalFrame; }

/// forward declaradion
class Signal;

/// signal key
typedef std::string SignalID;
/// signal return type
typedef void SignalRet;
/// signal argument
typedef XML::SignalFrame SignalArgs;
/// signal pointer
typedef Signal * SignalPtr;
/// signal pointer
typedef Signal *const SignalCPtr;

/// SignalHandler executes calls received as string by issuing signals to the slots
/// Slots may be:
///  * its own derived classes that regist  member functions to be called dynamically
///  * other classes that regist themselves to be notified when a signal is issued
///
/// @author Tiago Quintino
class Common_API SignalHandler {

public:

  /// storage type for signals
  typedef std::vector < SignalPtr >  storage_t;

public:

  ~SignalHandler();

  /// @return the signals
  const storage_t& signal_list () const;

  /// Access to signal by providing its name
  /// @throw SignalError if signal with name does not exist
  SignalPtr signal ( const SignalID& sname );

  /// Const access to a signal by providing its name
  /// @throw SignalError if signal with name does not exist
  SignalCPtr signal ( const SignalID& sname ) const;

  /// Calls the signal by providing its name and input
  SignalRet call_signal ( const SignalID& sname, SignalArgs& sinput );

  /// Calls the signal by providing its name and input
  SignalRet call_signal ( const SignalID& sname, std::vector<std::string>& sinput );

  /// Checks if a signal exists or not
  bool signal_exists ( const SignalID& sname ) const;

  /// Regist signal
  SignalPtr regist_signal ( const SignalID& sname );

  /// Unregist signal
  void unregist_signal ( const SignalID& sname );

public: // data

  /// storage of the signals
  storage_t  m_signals;

}; // SignalHandler

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_SignalHandler_hpp
