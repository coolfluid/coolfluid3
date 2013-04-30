// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <algorithm>

#include <boost/algorithm/string.hpp>

#include "common/Assertions.hpp"
#include "common/Signal.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalFrame.hpp"  // try forward declaration
#include "common/XML/SignalOptions.hpp"

#include "common/SignalHandler.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

using namespace XML;

////////////////////////////////////////////////////////////////////////////////

struct is_signal
{
  const std::string& name;
  is_signal(const std::string& n) : name(n) {}
  bool operator() ( Signal* s ) { return s->name() == name; }
};

SignalHandler::~SignalHandler()
{
  // deallocate all registered signals

  for( storage_t::iterator itr = m_signals.begin() ; itr != m_signals.end() ; ++itr )
    delete_ptr( *itr );
}

const SignalHandler::storage_t& SignalHandler::signal_list () const
{
  return m_signals;
}

SignalRet SignalHandler::call_signal ( const SignalID& sname, SignalArgs& sinput )
{
  return ( * signal( sname )->signal() ) ( sinput );
}

SignalRet SignalHandler::call_signal ( const SignalID& sname, std::vector<std::string>& sinput )
{
  SignalFrame frame;
  SignalFrame& options = frame.map( Protocol::Tags::key_options() );

  options.insert( sinput );

  return call_signal(sname, frame);
}

SignalPtr SignalHandler::signal ( const SignalID& sname )
{
  storage_t::iterator itr = std::find_if( m_signals.begin(), m_signals.end(), is_signal(sname) );
  if ( itr != m_signals.end() )
    return *itr;
  else
    throw SignalError ( FromHere(), "Signal with name \'" + sname + "\' does not exist" );
}

SignalCPtr SignalHandler::signal ( const SignalID& sname ) const
{
  storage_t::const_iterator itr = std::find_if( m_signals.begin(), m_signals.end(), is_signal(sname) );
  if ( itr != m_signals.end() )
    return *itr;
  else
    throw SignalError ( FromHere(), "Signal with name \'" + sname + "\' does not exist" );
}


bool SignalHandler::signal_exists ( const SignalID& sname ) const
{
  storage_t::const_iterator itr = std::find_if( m_signals.begin(), m_signals.end(), is_signal(sname) );
  return ( itr != m_signals.end() );
}


Signal& SignalHandler::regist_signal ( const SignalID& sname )
{
  // check sname complies with standard
  cf3_assert( boost::algorithm::all(sname,
                                   boost::algorithm::is_alnum() ||
                                   boost::algorithm::is_any_of("-_")) );

  storage_t::iterator itr = std::find_if( m_signals.begin(), m_signals.end(), is_signal(sname) );

  if ( itr == m_signals.end() )
  {
    SignalPtr psig ( new Signal( sname ) ); // allocate new signal

    m_signals.push_back( psig );

//    std::sort( m_signals.begin(), m_signals.end(), alphabetic_signal );

    return *psig;
  }
  else
    return **itr;
}


void SignalHandler::unregist_signal ( const SignalID& sname )
{
  storage_t::iterator itr = std::find_if( m_signals.begin(), m_signals.end(), is_signal(sname) );
  if ( itr != m_signals.end() )
  {
    delete_ptr( *itr );
    m_signals.erase(itr);
  }
}

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
