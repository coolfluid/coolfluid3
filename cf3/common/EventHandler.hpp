// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_EventHandler_hpp
#define cf3_common_EventHandler_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Signal.hpp"
#include "common/SignalHandler.hpp"

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

  static EventHandler& instance();

  /// Destructor
  ~EventHandler();

  /// Regists a signal on this EventHandler
  template < typename PTYPE, typename FTYPE >
  Signal* connect_to_event ( const std::string& sname, PTYPE* ptr, FTYPE pfunc )
  {
    ConnectionManager * mng = dynamic_cast<ConnectionManager*>(ptr);

    return &regist_signal ( sname ).connect( boost::bind ( pfunc, ptr, _1 ), mng );
  }

  /// raises an event and dispatches immedietly to all listeners
  void raise_event( const std::string& ename, SignalArgs& args);
  
private:
  /// Constructor
  EventHandler();

}; // class EventHandler

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_EventHandler_hpp
