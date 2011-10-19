// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>

#include "Common/Assertions.hpp"
#include "Common/Signal.hpp"
#include "Common/ConnectionManager.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalOptions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

using namespace XML;

////////////////////////////////////////////////////////////////////////////////

struct is_connection
{
  const std::string& name;
  is_connection(const std::string& n) : name(n) {}
  bool operator() ( Connection* s ) { return s->name() == name; }
};

ConnectionManager::~ConnectionManager()
{
  // disconnect all registered connections

  for( storage_t::iterator itr = m_connections.begin() ; itr != m_connections.end() ; ++itr )
    delete_ptr( *itr );
}

Connection* ConnectionManager::connection( const std::string& name )
{
  storage_t::iterator itr = std::find_if( m_connections.begin(), m_connections.end(), is_connection(name) );
  if ( itr != m_connections.end() )
    return *itr;
  else
    throw SignalError ( FromHere(), "Connection with name \'" + name + "\' does not exist" );

}

Connection* ConnectionManager::manage_connection( const std::string& name )
{
  storage_t::iterator itr = std::find_if( m_connections.begin(), m_connections.end(), is_connection(name) );
  if ( itr != m_connections.end() )
    return *itr;
  else
  {
    Connection* pc = new Connection(name);
    m_connections.push_back( pc );
    return pc;
  }
}

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
