// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_SignalHandler_hpp
#define CF_Common_SignalHandler_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/shared_ptr.hpp>
#include <boost/signals2/signal.hpp>

#include "Common/Exception.hpp"
#include "Common/NonInstantiable.hpp"
#include "Common/XmlSignature.hpp"
#include "Common/XML.hpp"

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

}; // Signal

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
    typedef XmlNode arg_t;
    /// signal type
    typedef boost::signals2::signal< Signal::return_t ( Signal::arg_t& ) >  type;
    /// signal pointer
    typedef boost::shared_ptr<type> Ptr;
    /// the boost signal object
    Ptr m_signal;
    /// signal description
    desc_t m_description;
    /// signal xml signature
    XmlSignature m_signature;
    /// signal readable name (used by the GUI). For exemple, if key is
    /// "set_options", readable should be "Set options".
    readable_t m_readable_name;
    /// if @c true, the signal is considered as read-only and might be called
    /// during another signal execution. Default value is @c false.
    bool m_is_read_only;
};

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

    /// Calls the signal by providing its name and input
    Signal::return_t call_signal ( const Signal::id_t& sname, Signal::arg_t& sinput );

    /// Regist signal
    Signal::Ptr regist_signal ( const Signal::id_t& sname, const Signal::desc_t& desc, const Signal::readable_t& readable_name = Signal::readable_t() );

    /// Checks whether a signal is read-only
    /// @throw SignalError if the signal does not exist
    bool is_signal_read_only( const Signal::id_t& sname ) const;

  protected: // functions

    /// Get a signal by providing its name
    Signal & signal ( const Signal::id_t& sname );

    /// Create a signal
    Signal::Ptr create_signal ( const Signal::id_t& sname,  const Signal::desc_t& desc, const Signal::readable_t& readable_name = Signal::readable_t() );

  protected: // data

    /// storage of the signals
    sigmap_t  m_signals;

}; // class SignalHandler

//#define ADD_SIGNAL_SLOT_TO_COMPONENT ( )


////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_SignalHandler_hpp
