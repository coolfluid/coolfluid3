// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_SignalHandler_hpp
#define CF_Common_SignalHandler_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/signals2/signal.hpp>

#include "Common/Exception.hpp"
#include "Common/XML/SignalFrame.hpp"

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

}; // SignalError

////////////////////////////////////////////////////////////////////////////////

/// Class that harbours the types handled by the SignalHandler
/// @author Tiago Quintino
struct Signal
{
    /// signal key
    typedef std::string id_t;
    /// signal description
    typedef std::string desc_t;
    /// signal readable name
    typedef std::string readable_t;
    /// signal return type
    typedef void return_t;
    /// signal argument
    typedef XML::SignalFrame arg_t;
    /// signal type
    typedef boost::signals2::signal< Signal::return_t ( Signal::arg_t& ) >  type;
    /// signal pointer
    typedef boost::shared_ptr<type> Ptr;
    /// the boost signal object
    Ptr signal_ptr;
    /// signal description
    desc_t description;
    /// signal readable name (used by the GUI). For exemple, if key is
    /// "set_options", readable should be "Set options".
    readable_t readable_name;
    /// if @c true, the signal is considered as read-only and might be called
    /// during another signal execution. Default value is @c false.
    bool is_read_only;

    bool is_hidden;

    Ptr signature;

}; // Signal

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
  typedef std::map < Signal::id_t , Signal >  sigmap_t;

  public:

    /// Get the list of signals and respective descriptions
    std::vector < Signal > list_signals () const;

    /// Access to signal by providing its name
    /// @throw SignalError if signal with name does not exist
    Signal& signal ( const Signal::id_t& sname );

    /// Const access to a signal by providing its name
    /// @throw SignalError if signal with name does not exist
    const Signal& signal ( const Signal::id_t& sname ) const;

    /// Calls the signal by providing its name and input
    Signal::return_t call_signal ( const Signal::id_t& sname, Signal::arg_t& sinput );

    /// Checks if a signal exists or not
    bool check_signal ( const Signal::id_t& sname );

    /// Regist signal
    Signal::Ptr regist_signal ( const Signal::id_t& sname, const Signal::desc_t& desc, const Signal::readable_t& readable_name = Signal::readable_t() );

  protected: // data

    /// storage of the signals
    sigmap_t  m_signals;

}; // SignalHandler


////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_SignalHandler_hpp
