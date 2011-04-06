// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>

#include "Common/Assertions.hpp"
#include "Common/Signal.hpp"
#include "Common/SignalHandler.hpp"
#include "Common/XML/Protocol.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

using namespace XML;

/// signal type
typedef boost::signals2::signal< SignalRet ( SignalArgs& ) >  SignalType;

////////////////////////////////////////////////////////////////////////////////

std::vector < SignalPtr > SignalHandler::list_signals () const
{
  std::vector < SignalPtr > result;
  for ( sigmap_t::const_iterator itr = m_signals.begin() ; itr != m_signals.end() ; ++itr )
    result.push_back ( itr->second ); // add a copy of the signal to the vector
  return result;
}

////////////////////////////////////////////////////////////////////////////////

const SignalHandler::sigmap_t& SignalHandler::signals_map () const
{
  return m_signals;
}

////////////////////////////////////////////////////////////////////////////////

SignalRet SignalHandler::call_signal ( const SignalID& sname, SignalArgs& sinput )
{
  return ( *signal ( sname )->signal ) ( sinput );
}

////////////////////////////////////////////////////////////////////////////////

SignalRet SignalHandler::call_signal ( const SignalID& sname, std::vector<std::string>& sinput )
{
  sigmap_t::iterator itr = m_signals.find(sname);
  if ( itr == m_signals.end() )
    throw SignalError ( FromHere(), "Signal with name \'" + sname + "\' does not exist" );
  
  SignalFrame frame;
  SignalFrame& options = frame.map( Protocol::Tags::key_options() );

  options.insert( sinput );

  call_signal(sname,frame);
}

////////////////////////////////////////////////////////////////////////////////

SignalPtr SignalHandler::signal ( const SignalID& sname )
{
  sigmap_t::iterator itr = m_signals.find(sname);
  if ( itr != m_signals.end() )
    return itr->second;
  else
    throw SignalError ( FromHere(), "Signal with name \'" + sname + "\' does not exist" );
}

////////////////////////////////////////////////////////////////////////////////

SignalCPtr SignalHandler::signal ( const SignalID& sname ) const
{
  sigmap_t::const_iterator itr = m_signals.find(sname);
  if ( itr != m_signals.end() )
    return itr->second;
  else
    throw SignalError ( FromHere(), "Signal with name \'" + sname + "\' does not exist" );
}

////////////////////////////////////////////////////////////////////////////////

bool SignalHandler::check_signal ( const SignalID& sname )
{
  return ( m_signals.find(sname) != m_signals.end() );
}

////////////////////////////////////////////////////////////////////////////////

SignalPtr SignalHandler::regist_signal ( const SignalID& sname,  const std::string& desc, const SignalName& readable_name )
{
  // check sname complies with standard
  cf_assert( boost::algorithm::all(sname,
                                   boost::algorithm::is_alnum() ||
                                   boost::algorithm::is_any_of("-_")) );
  
  std::string rname = readable_name;
  if (rname.empty())
    rname = sname;

  sigmap_t::iterator itr = m_signals.find (sname);

  if ( itr == m_signals.end() )
  {
    SignalPtr psig ( new Signal() );

    m_signals.insert( make_pair(sname, psig) );

    psig->signal = Signal::TypePtr( new SignalType() );
    psig->signature = Signal::TypePtr( new SignalType() );
    psig->description = desc;
    psig->readable_name = rname;
    psig->is_read_only = false;
    psig->is_hidden = false;

    return psig;
  }
  else
    return itr->second;
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
