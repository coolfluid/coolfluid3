// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/EventHandler.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

EventHandler::EventHandler ()
{}

EventHandler::~EventHandler()
{}

void EventHandler::raise_event( const std::string& ename, SignalArgs& args)
{
  sigmap_t::iterator itr = m_signals.find(ename);

  if ( itr == m_signals.end() ) return;

  // event exists so dispatch
  call_signal(ename, args);
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
