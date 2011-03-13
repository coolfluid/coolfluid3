// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_SignalHandler_hpp
#define CF_Common_SignalHandler_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/shared_ptr.hpp>

#include "Common/Exception.hpp"
#include "Common/XML/SignalFrame.hpp"  // try forward declaration

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/// forward declaradion
class Signal;

/// signal key
typedef std::string SignalID;
/// signal readable name
typedef std::string SignalName;
/// signal return type
typedef void SignalRet;
/// signal argument
typedef XML::SignalFrame SignalArgs;
/// signal pointer
typedef boost::shared_ptr<Signal> SignalPtr;
/// signal pointer
typedef boost::shared_ptr<Signal const> SignalCPtr;

/// SignalHandler executes calls received as string by issuing singals to the slots
/// Slots may be:
///  * its own derived classes that regist  member functions to be called dynamically
///  * other classes that regist themselves to be notified when a signal is issued
///
/// @author Tiago Quintino
class Common_API SignalHandler
{
  public:

    /// storage type for signals
  typedef std::map < SignalID , SignalPtr >  sigmap_t;

  public:

    /// Get the list of signals and respective descriptions
    std::vector < SignalPtr > list_signals () const;

    /// @return the signals
    const sigmap_t& signals_map () const;
    
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
    bool check_signal ( const SignalID& sname );

    /// Regist signal
    SignalPtr regist_signal ( const SignalID& sname, const std::string& desc, const SignalName& readable_name = SignalName() );

  public: // data

    /// storage of the signals
    sigmap_t  m_signals;

}; // SignalHandler

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_SignalHandler_hpp
