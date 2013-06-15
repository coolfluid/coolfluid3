// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Signal.hpp"

#include "common/ConnectionManager.hpp"

namespace cf3 {
namespace common {

  using namespace XML;

////////////////////////////////////////////////////////////////////////////////

SignalError::SignalError ( const common::CodeLocation& where, const std::string& what)
  : common::Exception(where, what, "SignalError")
{
}

SignalError::~SignalError() throw() {}

////////////////////////////////////////////////////////////////////////////////

Signal::Signal( const std::string& name ) :
  m_signal       ( new Signal::signal_type() ),
  m_signature    ( new Signal::signal_type() ),
  m_name         ( name ),
  m_description  (),
  m_pretty_name  (),
  m_is_read_only ( false ),
  m_is_hidden    ( false )
{
}

Signal::~Signal()
{
}

Signal& Signal::description( const std::string& desc )
{
  m_description = desc;
  return *this;
}

Signal& Signal::pretty_name( const std::string& name )
{
  m_pretty_name = name;
  return *this;
}

/// sets if it is read only signal
Signal& Signal::read_only( bool is )
{
  m_is_read_only = is;
  return *this;
}

Signal& Signal::hidden( bool is )
{
  m_is_hidden = is;
  return *this;
}

std::string Signal::name() const
{
  return m_name;
}

std::string Signal::description() const
{
  if( m_description.empty() )
    return pretty_name();
  else
    return m_description;
}

std::string Signal::pretty_name() const
{
  if( m_pretty_name.empty() )
    return name();
  else
    return m_pretty_name;
}

bool Signal::is_read_only() const { return m_is_read_only; }

bool Signal::is_hidden() const { return m_is_hidden; }

Signal& Signal::connect(const Signal::slot_type& subscriber)
{
  m_signal->connect( subscriber );
  return *this;
}

Signal& Signal::connect(const Signal::slot_type& subscriber, ConnectionManager* mng )
{
  Signal::connection_type conn = m_signal->connect( subscriber );
  mng->manage_connection( this->name() )
     ->connect( conn );
  return *this;
}


Signal& Signal::signature(const Signal::slot_type& subscriber)
{
  m_signature->connect( subscriber );
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

Connection::Connection( const std::string& cname ) :  m_name( cname ) {}

Connection::~Connection()
{
  disconnect();
}

Connection* Connection::connect( const Signal::connection_type& conn )
{
  m_connection = conn;
  return this;
}

void Connection::disconnect() { m_connection.disconnect(); }


////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
