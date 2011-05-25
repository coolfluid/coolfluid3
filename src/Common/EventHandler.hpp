// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_EventHandler_hpp
#define CF_Common_EventHandler_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Signal.hpp"
#include "Common/SignalHandler.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// Global Event Handler class
/// @todo make this a component?
/// @author Tiago Quintino
class Common_API EventHandler :
    public Common::SignalHandler,
    public boost::noncopyable {

public: // methods

  /// Constructor
  EventHandler();

  /// Destructor
  ~EventHandler();

  /// Regists a signal on this EventHandler
  template < typename PTYPE, typename FTYPE >
  Signal::ConnectionType connect_to_event ( const std::string& sname, PTYPE* ptr, FTYPE pfunc, const std::string& desc = "" )
  {
    return regist_signal ( sname , desc )->signal->connect ( boost::bind ( pfunc, ptr, _1 ) );
  }

  /// raises an event and dispatches immedietly to all listeners
  void raise_event( const std::string& ename, SignalArgs& args);

}; // class EventHandler

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_EventHandler_hpp
