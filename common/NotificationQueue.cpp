// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/URI.hpp"
#include "common/NotificationQueue.hpp"

using namespace cf3::common;

NotificationQueue::NotificationQueue(CRoot::Ptr root)
: m_sig_begin_flush(new SignalTypeFlush_t()),
m_event_signals()
{
  cf3_assert( root.get() != nullptr );

  root->add_notification_queue(this);
}

////////////////////////////////////////////////////////////////////////////////

NotificationQueue::~NotificationQueue()
{

}

////////////////////////////////////////////////////////////////////////////////

void NotificationQueue::add_notification ( const std::string & name,
                                           const URI & sender_path )
{
  cf3_assert( !name.empty() );

  m_notifications.push_back( std::pair<std::string, URI>(name, sender_path) );
}

////////////////////////////////////////////////////////////////////////////////

cf3::Uint NotificationQueue::nb_notifications ( const std::string & name ) const
{
  cf3::Uint count = 0;

  if ( name.empty() )
    count = m_notifications.size();
  else
  {
    std::vector< std::pair<std::string, URI> >::const_iterator it;

    for (it = m_notifications.begin() ; it != m_notifications.end() ; it++)
    {
      if(it->first == name)
        count++;
    }
  }

  return count;
}

////////////////////////////////////////////////////////////////////////////////

void NotificationQueue::flush()
{
  std::vector< std::pair<std::string, URI> >::iterator it;

  if( !m_notifications.empty() )
  {
    (*m_sig_begin_flush.get())(); // call the signal

    for(it = m_notifications.begin() ; it != m_notifications.end() ; it++)
    {
      EventSigsStorage_t::iterator itSig = m_event_signals.find(it->first);

      if(itSig != m_event_signals.end())
        (*itSig->second.get())(it->first, it->second);
    }

    m_notifications.clear();
  }
}
