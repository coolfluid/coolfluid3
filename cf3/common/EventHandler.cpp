// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/EventHandler.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

EventHandler& EventHandler::instance()
{
  static EventHandler event_handler;
  return event_handler;
}


EventHandler::EventHandler ()
{}

EventHandler::~EventHandler()
{}

void EventHandler::raise_event( const std::string& ename, SignalArgs& args)
{
  if ( signal_exists(ename) == false ) return;

  // event exists so dispatch

  call_signal(ename, args);
}

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
