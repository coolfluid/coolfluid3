// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_EventHandler_hpp
#define cf3_common_EventHandler_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Signal.hpp"
#include "Common/SignalHandler.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

/// Global Event Handler class
/// @todo make this a component?
/// @author Tiago Quintino
class Common_API EventHandler :
    public common::SignalHandler,
    public boost::noncopyable {

public: // methods

  /// Constructor
  EventHandler();

  /// Destructor
  ~EventHandler();

  /// Regists a signal on this EventHandler
  template < typename PTYPE, typename FTYPE >
  Signal* connect_to_event ( const std::string& sname, PTYPE* ptr, FTYPE pfunc )
  {
    ConnectionManager * mng = dynamic_cast<ConnectionManager*>(ptr);

    return regist_signal ( sname )->connect( boost::bind ( pfunc, ptr, _1 ), mng );
  }

  /// raises an event and dispatches immedietly to all listeners
  void raise_event( const std::string& ename, SignalArgs& args);

}; // class EventHandler

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_common_EventHandler_hpp
