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

std::vector < std::pair < Signal::id_t, Signal::desc_t > > SignalHandler::list_signals () const
{
  std::vector < std::pair < Signal::id_t, Signal::desc_t > > result;
  for ( sigmap_t::const_iterator itr = m_signals.begin() ; itr != m_signals.end() ; ++itr )
    result.push_back ( make_pair ( itr->first , itr->second.second ) );
  return result;
}

////////////////////////////////////////////////////////////////////////////////

Signal::return_t SignalHandler::call_signal ( const Signal::id_t& sname, Signal::arg_t& sinput )
{
  return ( *get_signal ( sname ) ) ( sinput );
}

////////////////////////////////////////////////////////////////////////////////

Signal::Ptr SignalHandler::get_signal ( const Signal::id_t& sname )
{
  sigmap_t::iterator itr = m_signals.find(sname);
  if ( itr != m_signals.end() )
    return itr->second.first;
  else
    throw SignalError ( FromHere(), "Signal with name \'" + sname + "\' does not exist" );
}

////////////////////////////////////////////////////////////////////////////////

Signal::Ptr SignalHandler::create_signal ( const Signal::id_t& sname,  const Signal::desc_t& desc )
{
  sigmap_t::iterator itr = m_signals.find (sname);
  if ( itr == m_signals.end() )
  {
    Signal::Ptr ptr ( new Signal::type() );
    m_signals.insert ( make_pair ( sname , make_pair ( ptr , desc ) )  );
    return ptr;
  }
  else
    throw SignalError ( FromHere(), "Signal with name \'" + sname + "\' already exists" );
}

////////////////////////////////////////////////////////////////////////////////

Signal::Ptr SignalHandler::regist_signal ( const Signal::id_t& sname,  const Signal::desc_t& desc )
{
  sigmap_t::iterator itr = m_signals.find (sname);
  if ( itr == m_signals.end() )
  {
    Signal::Ptr ptr ( new Signal::type() );
    m_signals.insert ( make_pair ( sname , make_pair ( ptr , desc ) )  );
    return ptr;
  }
  else
    return itr->second.first;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF
