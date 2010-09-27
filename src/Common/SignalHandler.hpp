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

}; // end of class Signal

////////////////////////////////////////////////////////////////////////////////

/// Class that harbours the types handled by the SignalHandler
/// @author Tiago Quintino
struct Signal : public NonInstantiable<Signal>
{
    /// signal key
    typedef std::string id_t;
    /// signal description
    typedef std::string desc_t;
    /// signal return type
    typedef void return_t;
    /// signal argument
    typedef XmlNode arg_t;
    /// signal type
    typedef boost::signals2::signal< Signal::return_t ( Signal::arg_t& ) >  type;
    /// signal pointer
    typedef boost::shared_ptr<type> Ptr;
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
    typedef std::map < Signal::id_t , std::pair< Signal::Ptr , Signal::desc_t > >  sigmap_t;

  public:

    /// Get the list of signals and respective descriptions
    std::vector < std::pair < Signal::id_t, Signal::desc_t > > list_signals () const;

    /// Calls the signal by providing its name and input
    Signal::return_t call_signal ( const Signal::id_t& sname, Signal::arg_t& sinput );

    /// Regist signal
    Signal::Ptr regist_signal ( const Signal::id_t& sname,  const Signal::desc_t& desc );

  protected: // functions

    /// Get a signal by providing its name
    Signal::Ptr signal ( const Signal::id_t& sname );

    /// Create a signal
    Signal::Ptr create_signal ( const Signal::id_t& sname,  const Signal::desc_t& desc );

  protected: // data

    /// storage of the signals
    sigmap_t  m_signals;

}; // class SignalHandler

//#define ADD_SIGNAL_SLOT_TO_COMPONENT ( )


////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_SignalHandler_hpp
