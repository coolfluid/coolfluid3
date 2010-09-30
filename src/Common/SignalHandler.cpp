// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/SignalHandler.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

SignalError::SignalError ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "SignalError")
{}

////////////////////////////////////////////////////////////////////////////////

std::vector < Signal > SignalHandler::list_signals () const
{
  std::vector < Signal > result;
  for ( sigmap_t::const_iterator itr = m_signals.begin() ; itr != m_signals.end() ; ++itr )
    result.push_back ( itr->second ); // add a copy of the signal to the vector
  return result;
}

////////////////////////////////////////////////////////////////////////////////

Signal::return_t SignalHandler::call_signal ( const Signal::id_t& sname, Signal::arg_t& sinput )
{
  return ( *signal ( sname ).m_signal ) ( sinput );
}

////////////////////////////////////////////////////////////////////////////////

Signal & SignalHandler::signal ( const Signal::id_t& sname )
{
  sigmap_t::iterator itr = m_signals.find(sname);
  if ( itr != m_signals.end() )
    return itr->second;
  else
    throw SignalError ( FromHere(), "Signal with name \'" + sname + "\' does not exist" );
}

////////////////////////////////////////////////////////////////////////////////

Signal::Ptr SignalHandler::create_signal ( const Signal::id_t& sname,  const Signal::desc_t& desc, const Signal::readable_t& readable_name )
{
  sigmap_t::iterator itr = m_signals.find (sname);
  if ( itr == m_signals.end() )
  {
    Signal signal;

    signal.m_signal = Signal::Ptr( new Signal::type() );
    signal.m_description = desc;
    signal.m_readable_name = readable_name;

    m_signals.insert ( make_pair ( sname , signal ) );
    return signal.m_signal;
  }
  else
    throw SignalError ( FromHere(), "Signal with name \'" + sname + "\' already exists" );
}

////////////////////////////////////////////////////////////////////////////////

Signal::Ptr SignalHandler::regist_signal ( const Signal::id_t& sname,  const Signal::desc_t& desc, const Signal::readable_t& readable_name )
{
  sigmap_t::iterator itr = m_signals.find (sname);
  if ( itr == m_signals.end() )
  {
    Signal signal;

    signal.m_signal = Signal::Ptr( new Signal::type() );
    signal.m_description = desc;
    signal.m_readable_name = readable_name;

    m_signals.insert ( make_pair ( sname , signal ) );
    return signal.m_signal;
  }
  else
    return itr->second.m_signal;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF
